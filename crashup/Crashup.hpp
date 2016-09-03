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
#endif

namespace crashup {

/**
 * Main class for controlling crash handling.
 */
class Crashup {

private:
  /// Directory for Crashup files (like minidumps).
  std::string data_directory;

  /// URL for Socorro upload server.
  std::string upload_url;

  /// Directory where current executable is placed
  /// Will look there for Crashpad Handler (if Windows)
  std::wstring executable_directory;

  /// Application name.
  std::string app_name;

  /// Application version.
  std::string app_version;

  /// Whether to limit minidump uploads to 1/hour.
  bool throttle = false;

#if defined(Q_OS_WIN32)
  /// Crashpad library driver.
  crashpad::CrashpadClient *_crashpad_client;
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
   * @param app_name: Application name.
   * @param app_version: Application version.
   * @param data_directory: Directory where minidumps will be placed.
   * @param upload_url: Address of Socorro upload server.
   */
  Crashup(std::string app_name, std::string app_version,
          std::string data_directory, std::string upload_url);

  /**
   * Initializes crash handling and uploading mechanisms.
   *
   * Run this after setting application name, version and other options.
   */
  void init();

  /**
   * Sets whether to limit number of uploads to 1/hour.
   *
   * Works only on Windows.
   */
  void setRateLimit(bool throttle);
};

} // namespace crashup
