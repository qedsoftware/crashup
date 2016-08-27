#pragma once
#include <QObject>
#include <QtCore/QProcess>
#include <map>
#include <string>
#include <vector>
#if defined(Q_OS_WIN32)
#include "../../google-crashpad/crashpad/client/simple_string_dictionary.h"
#endif

namespace crashpad {
class CrashpadClient;
}
namespace crash_handling {
class CrashHandler;
class CrashUploader;
}

namespace crashup {

class Crashup {

private:
  std::string working_dir, server_address;
  std::string app_name, app_version;
#if defined(Q_OS_WIN32)
  crashpad::CrashpadClient *_crashpad_client;
#elif defined(Q_OS_LINUX)
  crash_handling::CrashHandler *_crash_handler;
  crash_handling::CrashUploader *_crash_uploader;
#endif

  std::string makeInternalDirPath(const std::string &);
  void initCrashHandler();
  void initCrashUploader();

public:
  Crashup(std::string working_dir, std::string server_address);
  void init();

  void setAppName(const std::string &);
  void setAppVersion(const std::string &);

  void writeMinidump();
  void uploadPendingMinidumps();
};

} // namespace crashup
