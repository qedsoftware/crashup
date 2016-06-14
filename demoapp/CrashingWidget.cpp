#include "CrashingWidget.hpp"
#include "../../crashup/updates/Downloader.hpp"
#include "ui_CrashingWidget.h"

#include <signal.h>

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
  TestException *e = new TestException();
  e->raise();
}

void CrashingWidget::on_segfaultButton_clicked() {
  int *invalid_address = (int *)7;
  *invalid_address = 42;
}

void CrashingWidget::on_statsButton_clicked() {
  logEvent("message_entered_by_user",
           ui->textEdit->toPlainText().toUtf8().constData());
}

void CrashingWidget::on_downloadButton_clicked() {
  Downloader *d = new Downloader(ui->downloadDirectory->toPlainText(),
                                 ui->url->toPlainText());
  d->downloadFile();
}

void CrashingWidget::on_uploadButton_clicked() { uploadMinidumpsFromDir(); }
