#include <string>
#include <QWidget>

class WidgetTracker : public QObject {
  Q_OBJECT
public:
  WidgetTracker() {}

protected:
  bool eventFilter(QObject *obj, QEvent *event);

private:
  void updateObject(QObject *obj);
  void removeObject(QObject *obj);
  void updateObjectSubtree(QObject *obj);

  void printType(QObject *obj);
  void printGeometry(QObject *obj);
  void printProperties(QObject *obj);
};
