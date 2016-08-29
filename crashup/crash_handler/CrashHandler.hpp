#pragma once

#include <string>
#include <QCoreApplication>

namespace crash_handling {

class CrashHandlerPrivate;

/**
 * Controls crash handling on Linux.
 */
class CrashHandler {

public:
  /**
   * Returns instance of CrashHandler.
   */
  static CrashHandler *instance();

  /**
   * Initializes crash handling.
   *
   * @param report_minidumps_path: Path where minidumps will be stored.
   */
  void init(const std::string &report_minidumps_path);

  /**
   * Sets whether crashes should be reported to OS.
   *
   * @param toReport: False if we should never report crashes to OS,
   *                  True if we should report crashes to OS when
   *                  Exception handler returns failure.
   */
  void setReportCrashesToSystem(bool toReport);

  /**
   * Writes minidump on demand (the app does not crash).
   */
  bool writeMinidump();

private:
  CrashHandler();
  ~CrashHandler();
  CrashHandlerPrivate *pPrivCrashHandler;
  Q_DISABLE_COPY(CrashHandler)
};
}
