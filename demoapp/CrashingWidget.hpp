#include <QMainWindow>
#include <QException>
#include <QFileDialog>
#include <QString>
#include <QCoreApplication>

#include <string>
#include <functional>

class TestException : public QException {
public:
  void raise() const { throw * this; }
  TestException *clone() const { return new TestException(*this); }
};

namespace Ui {
class CrashingWidget;
}

class CrashingWidget : public QMainWindow {
  Q_OBJECT

public:
  CrashingWidget(std::function<void(std::string, std::string)> logEventCallback,
                 std::function<void()> uploadMinidump, QWidget *parent = 0);
  ~CrashingWidget();

private slots:
  void on_exceptionButton_clicked();

  void on_segfaultButton_clicked();

  void on_statsButton_clicked();

  void on_downloadButton_clicked();

  void on_uploadButton_clicked();

private:
  Ui::CrashingWidget *ui;
  std::function<void(std::string, std::string)> logEvent;
  std::function<void()> uploadMinidumpsFromDir;
};
