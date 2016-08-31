#pragma once
#include <QObject>
#include <QtCore/QProcess>
#include <map>
#include <string>
#include <vector>
#if defined(Q_OS_WIN32)
#include "client/simple_string_dictionary.h"
#endif

#if defined(Q_OS_WIN32)
namespace crashpad {
class CrashpadClient;
}
#elif defined(Q_OS_LINUX)
namespace crash_handling {
class CrashHandler;
class CrashUploader;
}
#endif

namespace crashup {

/**
 * Main class for controlling crash handling.
 */
class Crashup {

private:
  /// Directory for Crashup files (like minidumps).
  std::string working_dir;

  /// Network address for Socorro server.
  std::string server_address;

  /// Directory where current executable is placed
  /// Will look there for Crashpad Handler (if Windows)
  std::wstring executable_directory;

  /// Path to upload page on Socorro server.
  std::string upload_path = "/submit";

  /// Application name.
  std::string app_name;

  /// Application version.
  std::string app_version;

  /// Whether to limit minidump uploads to 1/hour.
  bool throttle = false;

#if defined(Q_OS_WIN32)
  /// Crashpad library driver.
  crashpad::CrashpadClient *_crashpad_client;
#elif defined(Q_OS_LINUX)
  /// Breakpad library driver.
  crash_handling::CrashHandler *_crash_handler;
  /// Class for handling minidumps upload.
  crash_handling::CrashUploader *_crash_uploader;
#endif

private:
  /**
   * Creates a directory in Crashup working directory.
   */
  std::string makeInternalDirPath(const std::string &);

  /**
   * Initializes crash handling.
   */
  void initCrashHandler();

  /**
   * Initializes minidump uploading.
   */
  void initCrashUploader();

public:
  /**
   * Creates new crash handling driver.
   *
   * @param working_directory: Directory where minidumps will be placed.
   * @param server_address: Address of Socorro server.
   */
  Crashup(std::string working_directory, std::string server_address);

  /**
   * Initializes crash handling and uploading mechanisms.
   *
   * Run this after setting application name, version and other options.
   */
  void init();

  /**
   * Sets application name.
   */
  void setAppName(const std::string &);

  /**
   * Sets application version.
   */
  void setAppVersion(const std::string &);

  /**
   * Sets whether to limit number of uploads to 1/hour.
   *
   * Works only on Windows.
   */
  void setRateLimit(bool throttle);

  /**
   * Writes minidump on demand.
   *
   * Works only on Linux.
   */
  void writeMinidump();

  /**
   * Uploads minidumps that weren't sent.
   *
   * Call this only on Linux - on Windows this is done automatically.
   */
  void uploadPendingMinidumps();
};

} // namespace crashup
