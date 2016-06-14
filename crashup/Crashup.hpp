#pragma once
#include <string>
#include <map>
#include <vector>
#include "updates/Updates.hpp"
#include "stats/Stats.hpp"
#include "SettingsWidget.hpp"
#include <QtCore/QProcess>

namespace crashpad {
class CrashpadClient;
};
namespace crash_handling {
class CrashHandler;
class CrashUploader;
};

namespace crashup {

class Crashup {

private:
  std::string working_dir, server_address;
  std::string app_name, app_version, app_platform;
  Stats _stats;
#if defined(Q_OS_WIN32)
  crashpad::CrashpadClient *_crashpad_client;
#elif defined(Q_OS_LINUX)
  crash_handling::CrashHandler *_crash_handler;
  crash_handling::CrashUploader *_crash_uploader;
#endif

  std::string makeInternalDirPath(const std::string &);

public:
  Crashup(std::string working_dir, std::string server_address);

  void setAppName(const std::string &);
  void setAppVersion(const std::string &);
  void setAppPlatform(const std::string &);

  /* initiates the _crashHandler */
  void initCrashHandler();
  /* writes a minidump whenever asked for */
  void writeMinidump();
  /* initiates the _crashUploader */
  void initCrashUploader();
  /* uploades minidump */
  void uploadPendingMinidumps();

  SettingsWidget &createSettingsWidget();
  std::string getFileRevisions();        // for stats and crash handler
  void setPollingInterval(int interval); // for SettingsWidget
};

}; // namespace crashup
