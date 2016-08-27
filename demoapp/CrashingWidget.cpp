#include "CrashingWidget.hpp"
#include "ui_CrashingWidget.h"
#include "MakeSegv.hpp"

#include <thread>

#include <signal.h>

#include <QMessageBox>

CrashingWidget::CrashingWidget(
    std::function<void(std::string, std::string)> logEventCallback,
    std::function<void()> uploadMinidumpsFromDir, QWidget *parent)
    : QMainWindow(parent), ui(new Ui::CrashingWidget),
      logEvent(logEventCallback) {
  ui->setupUi(this);
  this->uploadMinidumpsFromDir = uploadMinidumpsFromDir;
}

CrashingWidget::~CrashingWidget() { delete ui; }

void CrashingWidget::on_exceptionButton_clicked() {
  if (ui->separateThread->isChecked()) {
    thread_make_cxxexception();
  } else {
    if (ui->useMakeSegv->isChecked()) {
      make_cxxexception();
    } else {
      TestException *e = new TestException();
      e->raise();
    }
  }
}

void CrashingWidget::on_segfaultButton_clicked() {
  if (ui->separateThread->isChecked()) {
    thread_make_segfault();
  } else {
    if (ui->useMakeSegv->isChecked()) {
      make_segfault();
    } else {
      int *invalid_address = (int *)7;
      *invalid_address = 42;
    }
  }
}

void CrashingWidget::on_statsButton_clicked() {
  QMessageBox mbox;
  mbox.setText("Sending stats is unavailable...");
  mbox.exec();
}

void CrashingWidget::on_downloadButton_clicked() {
  QMessageBox mbox;
  mbox.setText("Downloading is unavailable...");
  mbox.exec();
}

void CrashingWidget::on_uploadButton_clicked() { uploadMinidumpsFromDir(); }
