#include <QDebug>
#include <QtGlobal>
#include <QEvent>
#include <QHideEvent>
#include <QChildEvent>
#include <QMoveEvent>
#include <QString>
#include <QMetaProperty>
#include <QMetaObject>
#include "WidgetTracker.hpp"

void WidgetTracker::printProperties(QObject *obj) {
  if(obj->objectName() == "")
    return;
  const QMetaObject *metaobject = obj->metaObject();
  int count = metaobject->propertyCount();
  for (int i=0; i<count; ++i) {
    QMetaProperty metaproperty = metaobject->property(i);
    const char *name = metaproperty.name();
    QVariant value = obj->property(name);
    QString str_value = value.toString();
    if(
      std::string(value.typeName()) == "QString"
      && std::string(name) != "html"
      && std::string(name) != "objectName"
      && str_value != ""
    )
    qDebug().nospace() << "WidgetTracker: "
      << obj->objectName().toUtf8().constData()
      << "." << name << " = " << str_value;
  }
}

void WidgetTracker::printGeometry(QObject *obj) {
  if(obj->objectName() == "")
    return;
  if(QWidget *w = dynamic_cast<QWidget*>(obj)) {
    QRect rect = w->geometry();
    rect.moveTopLeft(w->mapToGlobal(QPoint(0, 0)));
    qDebug().nospace() << "WidgetTracker: "
      << obj->objectName().toUtf8().constData()
      << "." << "left" << " = " << rect.left();
    qDebug().nospace() << "WidgetTracker: "
      << obj->objectName().toUtf8().constData()
      << "." << "top" << " = " << rect.top();
    qDebug().nospace() << "WidgetTracker: "
      << obj->objectName().toUtf8().constData()
      << "." << "right" << " = " << rect.right();
    qDebug().nospace() << "WidgetTracker: "
      << obj->objectName().toUtf8().constData()
      << "." << "bottom" << " = " << rect.bottom();
  }
}

void WidgetTracker::printType(QObject *obj) {
  if(obj->objectName() == "")
    return;
  qDebug().nospace() << "WidgetTracker: "
    << obj->objectName().toUtf8().constData()
    << "." << "className" << " = " << QString(obj->metaObject()->className());
}

void WidgetTracker::updateObject(QObject *obj) {
  printType(obj);
  printProperties(obj);
  printGeometry(obj);
}

void WidgetTracker::removeObject(QObject *obj) {
  if(obj->objectName() == "")
    return;
  qDebug().nospace() << "WidgetTracker: "
    << "del " << obj->objectName().toUtf8().constData();
}

void WidgetTracker::updateObjectSubtree(QObject *obj) {
  QList<QWidget *> widgets = obj->findChildren<QWidget *>();
  for(QWidget *w : widgets) {
    updateObject(w);
  }
}

bool WidgetTracker::eventFilter(QObject *obj, QEvent *event) {
  if(QChildEvent *e = dynamic_cast<QChildEvent*>(event)) {
    if(e->type() == QEvent::ChildAdded) {
      e->child()->installEventFilter(this);
    }
  }
  if(event->type() == QEvent::Polish) {
    updateObject(obj);
  }
  if(event->type() == QEvent::Show) {
    updateObject(obj);
  }
  if(QMoveEvent *e = dynamic_cast<QMoveEvent*>(event)) {
    if(QWidget *w = dynamic_cast<QWidget*>(obj))
    if(
        e->type() == QEvent::Move
        && (e->oldPos() != e->pos() || w->isWindow())
      ) {
      /*
      qDebug() << "<MOVE EVENT>";
      qDebug() << e->oldPos();
      qDebug() << e->pos();
      // */
      updateObjectSubtree(obj);
      // qDebug() << "</MOVE EVENT>";
    }
  }
  if(event->type() == QEvent::Hide) {
    removeObject(obj);
  }
  return QObject::eventFilter(obj, event);
}
