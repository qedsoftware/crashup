#if !defined(CRASHHANDLER_HPP)
#define CRASHHANDLER_HPP

#include <string>
#include <QCoreApplication>

namespace crash_handling {

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
