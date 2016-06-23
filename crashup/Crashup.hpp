#pragma once
#include "SettingsWidget.hpp"
#include "stats/Stats.hpp"
#include "updates/Updates.hpp"
#include <QObject>
#include <QTimer>
#include <QtCore/QProcess>
#include <map>
#include <string>
#include <vector>
#if defined(Q_OS_WIN32)
#include "../../google-crashpad/crashpad/client/simple_string_dictionary.h"
#endif

namespace crashpad {
class CrashpadClient;
};
namespace crash_handling {
class CrashHandler;
class CrashUploader;
};

namespace crashup {

#if defined(Q_OS_WIN32)
class Uptime : public QObject {
  Q_OBJECT

public:
  Uptime();
  crashpad::SimpleStringDictionary *getAnnotations();
public slots:
  void updateUptime();

private:
  int uptime;
  std::string uptime_str;
  crashpad::SimpleStringDictionary *annotations;
  QTimer *timer;
};
#endif

class Crashup {

private:
  std::string working_dir, server_address;
  std::string app_name, app_version, app_platform;
  Stats _stats;
#if defined(Q_OS_WIN32)
  crashpad::CrashpadClient *_crashpad_client;
  Uptime uptime;
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

  void initCrashHandler();
  void writeMinidump();
  void initCrashUploader();
  void uploadPendingMinidumps();
  void sendUsageReport(const std::string &, const std::string &);

  SettingsWidget &createSettingsWidget();
  std::string getFileRevisions();        // for stats and crash handler
  void setPollingInterval(int interval); // for SettingsWidget
};

}; // namespace crashup
