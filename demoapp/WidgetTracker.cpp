#include <Qt>
#include <QtGlobal>
#include <QMetaProperty>
#include <QMetaObject>
#include <QString>
#include <QDebug>
#include <QEvent>
#include <QHideEvent>
#include <QChildEvent>
#include <QMoveEvent>

#include "WidgetTracker.hpp"

WidgetTracker::WidgetTracker(QApplication *a) : app(a) {
  // use queued connection to trigger the eventLoopStartedSlot
  // so that it is executed when the QApplication::exec starts processing events
  // - exactly at point when we think application is initialized and ready
  // for the testing script to be interacted with. we also then update the
  // positions of all widgets, when we are certain they are correct
  // (application is initialized)
  connect(this, SIGNAL(eventLoopStartedSignal()), this,
          SLOT(eventLoopStartedSlot()), Qt::QueuedConnection);
  emit eventLoopStartedSignal();
  // an entry-point where we plug in to the object tree to intercept events and
  // report to testing script when any of widgets is moved, hidden, etc.
  a->installEventFilter(this);
}

void WidgetTracker::printProperties(QObject *obj) {
  if (obj->objectName() == "")
    return;
  const QMetaObject *metaobject = obj->metaObject();
  int count = metaobject->propertyCount();
  for (int i = 0; i < count; ++i) {
    QMetaProperty metaproperty = metaobject->property(i);
    const char *name = metaproperty.name();
    QVariant value = obj->property(name);
    QString str_value = value.toString();
    if (std::string(value.typeName()) == "QString" &&
        std::string(name) != "html" && std::string(name) != "objectName" &&
        str_value != "")
      qDebug().nospace() << "WidgetTracker: "
                         << obj->objectName().toUtf8().constData() << "."
                         << name << " = " << str_value;
  }
}

void WidgetTracker::printGeometry(QObject *obj) {
  if (obj->objectName() == "")
    return;
  if (QWidget *w = dynamic_cast<QWidget *>(obj)) {
    QRect rect = w->geometry();
    rect.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
    qDebug().nospace() << "WidgetTracker: "
                       << obj->objectName().toUtf8().constData() << "."
                       << "left"
                       << " = " << rect.left();
    qDebug().nospace() << "WidgetTracker: "
                       << obj->objectName().toUtf8().constData() << "."
                       << "top"
                       << " = " << rect.top();
    qDebug().nospace() << "WidgetTracker: "
                       << obj->objectName().toUtf8().constData() << "."
                       << "right"
                       << " = " << rect.right();
    qDebug().nospace() << "WidgetTracker: "
                       << obj->objectName().toUtf8().constData() << "."
                       << "bottom"
                       << " = " << rect.bottom();
  }
}

void WidgetTracker::printType(QObject *obj) {
  if (obj->objectName() == "")
    return;
  qDebug().nospace() << "WidgetTracker: "
                     << obj->objectName().toUtf8().constData() << "."
                     << "className"
                     << " = " << QString(obj->metaObject()->className());
}

void WidgetTracker::updateObject(QObject *obj) {
  printType(obj);
  printProperties(obj);
  printGeometry(obj);
}

void WidgetTracker::removeObject(QObject *obj) {
  if (obj->objectName() == "")
    return;
  qDebug().nospace() << "WidgetTracker: "
                     << "del " << obj->objectName().toUtf8().constData();
}

void WidgetTracker::updateObjectSubtree(QObject *obj) {
  QList<QWidget *> widgets = obj->findChildren<QWidget *>();
  for (QWidget *w : widgets) {
    updateObject(w);
  }
}

void WidgetTracker::updateAllWidgets() {
  for (QWidget *w : all_widgets) {
    updateObject(w);
  }
}

void WidgetTracker::eventLoopStartedSlot() {
  // event loop started, all windows have now correct positions
  // so we need to notify the testing script of them
  // because QWidget::mapToGlobal works incorrectly before
  // main window creation!!!
  updateAllWidgets();
  // notify testing script that the application is ready to interact with
  qDebug().nospace() << "WidgetTracker: "
                     << "READY";
}

void WidgetTracker::removeObjectFromListSlot(QObject *obj) {
  if (QWidget *w = dynamic_cast<QWidget *>(obj)) {
    all_widgets.removeAll(w);
  }
}

bool WidgetTracker::eventFilter(QObject *obj, QEvent *event) {
  if (QChildEvent *e = dynamic_cast<QChildEvent *>(event)) {
    // propagate the event filter to all child objects:
    if (e->type() == QEvent::ChildAdded) {
      e->child()->installEventFilter(this);
      if (QWidget *w = dynamic_cast<QWidget *>(e->child())) {
        // store all widgets in order to report their positions later
        // in function updateAllWidgets()
        all_widgets.push_back(w);
        // on destruction remove it from the list to avoid segfaults
        connect(w, SIGNAL(destroyed(QObject *)), this,
                SLOT(removeObjectFromListSlot(QObject *)),
                Qt::DirectConnection);
      }
    }
  }
  if (event->type() == QEvent::Show || event->type() == QEvent::Polish) {
    if (QWidget *w = dynamic_cast<QWidget *>(obj)) {
      if (w->isWindow()) {
        // if it is a window, all the widgets need to be updated with the new
        // coordinates (formerly they had coordinates relative to the window,
        // when window is shown, they get coordinates relative to the screen)
        updateAllWidgets();
      } else {
        updateObject(obj);
      }
    } else {
      updateObject(obj);
    }
  }
  if (QMoveEvent *e = dynamic_cast<QMoveEvent *>(event)) {
    if (QWidget *w = dynamic_cast<QWidget *>(obj))
      if (e->type() == QEvent::Move) {
        // if widget was moved, all it's children were also moved
        updateObjectSubtree(obj);
      }
  }
  if (event->type() == QEvent::Hide) {
    removeObject(obj);
  }
  if (event->type() == QEvent::DeferredDelete) {
    removeObjectFromListSlot(obj);
  }
  return QObject::eventFilter(obj, event);
}
