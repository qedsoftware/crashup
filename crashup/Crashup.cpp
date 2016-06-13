#include "Crashup.hpp"
#include <QDir>
#include <QString>
#include <QDebug>
#include <iostream>
#include <cstdlib>

#if defined(Q_OS_WIN32)
#include "../../google-crashpad/crashpad/client/crashpad_client.h"
#include "../../google-crashpad/crashpad/third_party/mini_chromium/mini_chromium/base/files/file_path.h"
#include "../../google-crashpad/crashpad/client/settings.h"
#include "../../google-crashpad/crashpad/client/crash_report_database.h"
#elif defined(Q_OS_LINUX)
#include "crash_handler/CrashHandler.hpp"
#include "crash_handler/CrashUploader.hpp"
#endif

namespace crashup {

const std::string Crashup::report_minidumps_relative_path = "minidumps";

Crashup::Crashup(std::string working_dir, std::string server_address)
    : _stats(working_dir, server_address) {
  this->report_minidumps_dirpath = "";
  this->working_dir = working_dir;
  this->server_address = server_address;
#if defined(Q_OS_WIN32)
  this->_crashpad_client = nullptr;
#elif defined(Q_OS_LINUX)
  this->_crashHandler = nullptr;
  this->_crashUploader = nullptr;
#endif
}

Stats &Crashup::stats() { return _stats; }

bool Crashup::checkRelativeDirpath(std::string &dirpath) {
  QString final_dir_path = QDir(QString::fromStdString(this->working_dir))
                               .filePath(QString::fromStdString(dirpath));
  QDir final_dir = QDir(final_dir_path);
  if (!final_dir.exists()) {
    // create the dir in case it doesn't exist
    QDir().mkpath(final_dir_path);
  }

  if (!(QFileInfo(final_dir_path).isDir() &&
        QFileInfo(final_dir_path).isWritable())) {
    // if the requested dir still doesn't exist or there is no permittion to
    // write in it -- throw exception
    return false;
  }

  dirpath = final_dir.absolutePath().toUtf8().constData();
  return true;
}

void Crashup::initCrashHandler() {
#if defined(Q_OS_WIN32)
  // initialize crash handler
  this->_crashpad_client = new crashpad::CrashpadClient();
  const std::wstring handler(L"C:\\Users\\Administrator\\Documents\\desktop-"
                             L"crashup\\google-crashpad\\crashpad\\out\\Debug_"
                             L"x64\\crashpad_handler.exe");
  const std::wstring crashdb(L".\\crashdb");
  int res = _crashpad_client->StartHandler(
      base::FilePath(handler), base::FilePath(crashdb), this->server_address,
      // data to send with POST requests uploading minidumps:
      std::map<std::string, std::string>{
          {"app_name", "demoapp"},
          {"app_version", "0.42"},
          {"app_platform", "windows"},
          {"mac_address", "<not available yet>"}},
      // additional options for 'crashpad_handler' executable:
      std::vector<std::string>{"--no-rate-limit"}, false);
  // TODO make --no-rate-limit configurable from outside
  if (!res) {
    std::cerr << "StartHandler" << std::endl;
    exit(1);
  }
  res = _crashpad_client->UseHandler();
  if (!res) {
    std::cerr << "UseHandler" << std::endl;
    exit(1);
  }
  // turn on minidump uploads in crashdb settings
  res = crashpad::CrashReportDatabase::Initialize(base::FilePath(crashdb))
            ->GetSettings()
            ->SetUploadsEnabled(true);
  if (!res) {
    std::cerr << "SetUploadsEnabled" << std::endl;
    exit(1);
  }

#elif defined(Q_OS_LINUX)
  std::string report_minidumps_absolute_dirpath =
      Crashup::report_minidumps_relative_path;
  if (checkRelativeDirpath(report_minidumps_absolute_dirpath)) {
    /* report_minidumps_absolute_dirpath is a valid absolute path now */
    this->report_minidumps_dirpath = report_minidumps_absolute_dirpath;
  } else {
    throw CrashupInitMinidumpDirpathException();
  }
  this->_crashHandler = crash_handling::CrashHandler::instance();
  this->_crashHandler->init(report_minidumps_absolute_dirpath);
// this->_crashHandler->setReportCrashesToSystem(true);			/* false
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
  if (_crashHandler != nullptr)
    _crashHandler->writeMinidump();
  else
#endif
    throw CrashupInitializationException("CrashHandler not initialized.");
}

/* initializing Crash Uploader does not require initizing Crash Handler */
/* first (or even at all)                                               */
void Crashup::initCrashUploader(const std::string &product_name,
                                const std::string &product_version) {
#if defined(Q_OS_LINUX)
  std::string report_minidumps_absolute_dirpath =
      Crashup::report_minidumps_relative_path;
  if (checkRelativeDirpath(report_minidumps_absolute_dirpath)) {
    /* report_minidumps_absolute_dirpath is a valid absolute path now */
    this->_crashUploader = new crash_handling::CrashUploader(
        QString::fromStdString(product_name),
        QString::fromStdString(product_version),
        QString::fromStdString(this->server_address),
        QString::fromStdString(report_minidumps_absolute_dirpath));
  } else {
    throw CrashupInitMinidumpDirpathException();
  }
#endif
}

void Crashup::uploadPendingMinidumps() {
#if defined(Q_OS_LINUX)
  if (_crashUploader != nullptr)
    _crashUploader->uploadPendingMinidumps();
  else
#endif
    throw CrashupInitializationException("CrashUploader not initialized.");
}

}; // namespace crashup
