#include "Crashup.hpp"
#include "exceptions.hpp"
#include <QDebug>
#include <QDir>
#include <QString>
#include <cstdlib>
#include <iostream>

#if defined(Q_OS_WIN32)
#include "../../google-crashpad/crashpad/client/crash_report_database.h"
#include "../../google-crashpad/crashpad/client/crashpad_client.h"
#include "../../google-crashpad/crashpad/client/crashpad_info.h"
#include "../../google-crashpad/crashpad/client/settings.h"
#include "../../google-crashpad/crashpad/third_party/mini_chromium/mini_chromium/base/files/file_path.h"
#elif defined(Q_OS_LINUX)
#include "crash_handler/CrashHandler.hpp"
#include "crash_handler/CrashUploader.hpp"
#endif

namespace crashup {

#if defined(Q_OS_WIN32)
Uptime::Uptime() {
  annotations = new crashpad::SimpleStringDictionary;
  uptime = 0;
  uptime_str = "0";
  timer = new QTimer(this);
  connect(timer, SIGNAL(timeout()), this, SLOT(updateUptime()));
  timer->start(1000);
  annotations->SetKeyValue("uptime", uptime_str.c_str());
}

crashpad::SimpleStringDictionary *Uptime::getAnnotations() {
  return annotations;
}

void Uptime::updateUptime() {
  ++uptime;
  uptime_str = std::to_string(uptime);
  annotations->SetKeyValue("uptime", uptime_str.c_str());
}
#endif

Crashup::Crashup(std::string working_dir, std::string server_address)
    : _stats(working_dir, server_address) {
  this->working_dir = working_dir;
  this->server_address = server_address;
#if defined(Q_OS_WIN32)
  this->_crashpad_client = nullptr;
#elif defined(Q_OS_LINUX)
  this->_crash_handler = nullptr;
  this->_crash_uploader = nullptr;
#endif
}

void Crashup::setAppName(const std::string &name) { this->app_name = name; }
void Crashup::setAppVersion(const std::string &version) {
  this->app_version = version;
}
void Crashup::setAppPlatform(const std::string &platform) {
  this->app_platform = platform;
}

std::string Crashup::makeInternalDirPath(const std::string &dirpath) {
  QString final_dir_path = QDir(QString::fromStdString(this->working_dir))
                               .filePath(QString::fromStdString(dirpath));
  QDir final_dir = QDir(final_dir_path);
  if (!final_dir.exists()) {
    // create the dir in case it doesn't exist
    QDir().mkpath(final_dir_path);
  }

  if (!(QFileInfo(final_dir_path).isDir() &&
        QFileInfo(final_dir_path).isWritable())) {
    // if the requested dir still doesn't exist or there is no permission to
    // write in it -- throw exception
    throw CrashupInitializationException(
        ("Cannot create '" + dirpath + "' directory for internal use.")
            .c_str());
  }
  return final_dir.absolutePath().toUtf8().constData();
}

void Crashup::initCrashHandler() {
#if defined(Q_OS_WIN32)
  // initialize crash handler
  this->_crashpad_client = new crashpad::CrashpadClient();
  std::wstring handler(L"C:\\Users\\Administrator\\Documents\\desktop-"
                       L"crashup\\google-crashpad\\crashpad\\out\\Debug_"
                       L"x64\\crashpad_handler.exe");
  std::wstring crashdb;
  std::copy(this->working_dir.begin(), this->working_dir.end(),
            std::back_inserter(crashdb));
  crashdb += L"\\crashdb";
  int res = _crashpad_client->StartHandler(
      base::FilePath(handler), base::FilePath(crashdb),
      this->server_address + "/api/upload_minidump/",
      // data to send with POST requests uploading minidumps:
      std::map<std::string, std::string>{
          {"app_name", this->app_name},
          {"app_version", this->app_version},
          {"app_platform", this->app_platform},
          {"mac_address", "<not available yet>"}},
      // additional options for 'crashpad_handler' executable:
      std::vector<std::string>{"--no-rate-limit"}, false);
  // TODO make --no-rate-limit configurable from outside
  // --no-rate-limit  <-- disable throttling upload attempts to 1/hour
  crashpad::CrashpadInfo::GetCrashpadInfo()->set_simple_annotations(
      uptime.getAnnotations());
  if (!res) {
    throw CrashupInitializationException("CrashpadClient::StartHandler failed");
  }
  res = _crashpad_client->UseHandler();
  if (!res) {
    throw CrashupInitializationException("CrashpadClient::UseHandler failed");
  }
#elif defined(Q_OS_LINUX)
  std::string minidumps_abspath = makeInternalDirPath("minidumps");
  this->_crash_handler = crash_handling::CrashHandler::instance();
  this->_crash_handler->init(minidumps_abspath);
// this->_crash_handler->setReportCrashesToSystem(true); /*
// false
// --> every crash is treated as successfully handled by breakpad (default
// choice here) */
/* true  --> crashes unsuccessfully handled by breakpad (ExceptionHandler
* returning success = false */
/* 			 to the callback function) are treated as unhandled and
* can
* be subsequently handled */
/*			 by another handler, they are being reported to the
* system
*/
#elif defined(Q_OS_MAC)
  throw TODOException("Crashup::initCrashHandler -- no OS_MAC support")
#endif
}

void Crashup::writeMinidump() {
#if defined(Q_OS_LINUX)
  if (_crash_handler != nullptr)
    _crash_handler->writeMinidump();
  else
    throw CrashupInitializationException("CrashHandler not initialized.");
#else
  throw TODOException(
      "On-demand minidump writing is not implemented on other OS.");
#endif
}

void Crashup::initCrashUploader() {
#if defined(Q_OS_WIN32)
  std::wstring crashdb;
  std::copy(this->working_dir.begin(), this->working_dir.end(),
            std::back_inserter(crashdb));
  crashdb += L"\\crashdb";
  // turn on minidump uploads in crashdb settings
  int res = crashpad::CrashReportDatabase::Initialize(base::FilePath(crashdb))
                ->GetSettings()
                ->SetUploadsEnabled(true);
  if (!res) {
    throw CrashupInitializationException(
        "crashpad::CrashReportDatabase::Settings::SetUploadsEnabled failed");
  }
#elif defined(Q_OS_LINUX)
  std::string minidumps_abspath = makeInternalDirPath("minidumps");
  this->_crash_uploader = new crash_handling::CrashUploader(
      QString::fromStdString(this->app_name),
      QString::fromStdString(this->app_version),
      QString::fromStdString(this->server_address + "/api/upload_minidump/"),
      QString::fromStdString(minidumps_abspath));
#elif defined(Q_OS_MAC)
  throw TODOException("Crashup::initCrashUploader -- no OS_MAC support")
#endif
}

void Crashup::uploadPendingMinidumps() {
#if defined(Q_OS_LINUX)
  if (_crash_uploader != nullptr)
    _crash_uploader->uploadPendingMinidumps();
  else
#endif
    throw CrashupInitializationException("CrashUploader not initialized.");
}

void Crashup::sendUsageReport(const std::string &t, const std::string &d) {
  this->_stats.send(std::map<std::string, std::string>{
      {"event_type", t},
      {"event_data", d},
      {"app_name", this->app_name},
      {"app_version", this->app_version},
      {"app_platform", this->app_platform},
      {"mac_address", "<not available yet>"},
  });
}
}; // namespace crashup
