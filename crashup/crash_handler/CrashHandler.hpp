#if !defined(CRASHHANDLER_HPP)
#define CRASHHANDLER_HPP

#include <string>
#include <QCoreApplication>

#if defined(Q_OS_LINUX)
#include "client/linux/handler/exception_handler.h"
#elif defined(Q_OS_WIN32)
#include "client/windows/handler/exception_handler.h"
#endif

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
};
#endif
