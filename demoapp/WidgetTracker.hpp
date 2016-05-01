#include <QWidget>
#include <QApplication>

class WidgetTracker : public QObject {
  Q_OBJECT

public:
  WidgetTracker(QApplication *a);
  void updateObject(QObject *obj);
  void removeObject(QObject *obj);
  void updateObjectSubtree(QObject *obj);
  void updateAllWidgets();

signals:
  void eventLoopStartedSignal();

private slots:
  void removeObjectFromListSlot(QObject *obj);
  void eventLoopStartedSlot();

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  void printType(QObject *obj);
  void printGeometry(QObject *obj);
  void printProperties(QObject *obj);

  QApplication *app;
  QList<QWidget *> all_widgets;
};
