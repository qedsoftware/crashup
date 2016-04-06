#pragma once
#include <string>
#include <QCoreApplication>
#include "client/linux/handler/exception_handler.h"

namespace crashhandler {

class CrashHandlerPrivate;
class CrashHandler {

public:
  static CrashHandler *instance();
  void init(const std::string &report_minidumps_path);
  void setReportCrashesToSystem(bool toReport);
  bool writeMinidump();

private:
  CrashHandler();
  ~CrashHandler();
  CrashHandlerPrivate *pPrivCrashHandler;
  Q_DISABLE_COPY(CrashHandler)
};
}