#include "CrashingWidget.hpp"
#include "ui_CrashingWidget.h"

#include <signal.h>

CrashingWidget::CrashingWidget(std::function<void(std::string, std::string)> logEvent, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CrashingWidget)
{
    ui->setupUi(this);
    this->logEvent = logEvent;
}

CrashingWidget::~CrashingWidget()
{
    delete ui;
}

void CrashingWidget::on_exceptionButton_clicked()
{
    TestException *e = new TestException();
    e->raise();
}

void CrashingWidget::on_segfaultButton_clicked()
{
    ::raise(SIGSEGV);
}

void CrashingWidget::on_statsButton_clicked()
{
    logEvent("sample_event", ui->textEdit->toPlainText().toStdString());
}
