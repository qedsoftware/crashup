#pragma once
#include <string>
#include <map>
#include <vector>
#include "updates/Updates.hpp"
#include "stats/Stats.hpp"
#include "SettingsWidget.hpp"
#include "crash_handler/CrashHandler.hpp"
#include "crash_handler/CrashUploader.hpp"
#if defined(Q_OS_WIN32)
#include "../../google-crashpad/crashpad/client/crashpad_client.h"
#include "../../google-crashpad/crashpad/third_party/mini_chromium/mini_chromium/base/files/file_path.h"
#endif
#include <QtCore/QProcess>
#include "exceptions.hpp"

namespace crashup {

class Crashup {

private:
  std::string working_dir, server_address;
  std::string report_minidumps_dirpath;
  Stats _stats;
#if defined(Q_OS_WIN32)
  crashpad::CrashpadClient *_crashpad_client;
#endif
  crash_handling::CrashHandler *_crashHandler;
  crash_handling::CrashUploader *_crashUploader;
  static const std::string report_minidumps_relative_path;

  bool checkRelativeDirpath(std::string &dirpath);

public:
  Crashup(std::string working_dir, std::string server_address);
  Updates &updates();
  Stats &stats();

  /* initiates the _crashHandler */
  void initCrashHandler();
  /* writes a minidump whenever asked for */
  void writeMinidump();
  /* initiates the _crashUploader */
  void initCrashUploader(const std::string &product_name,
                         const std::string &product_version);
  /* uploades minidump */
  void uploadPendingMinidumps();

  SettingsWidget &createSettingsWidget();
  std::string getFileRevisions();        // for stats and crash handler
  void setPollingInterval(int interval); // for SettingsWidget
};

}; // namespace crashup
