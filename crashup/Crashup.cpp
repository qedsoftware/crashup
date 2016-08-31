#include "Crashup.hpp"
#include "exceptions.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QString>
#include <cstdlib>
#include <iostream>

#if defined(Q_OS_WIN32)
// TODO: fix this when Hunter :: Crashpad will be working.
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/crashpad_info.h"
#include "client/settings.h"
#include "base/files/file_path.h"
#elif defined(Q_OS_LINUX)
#include "crash_handler/CrashHandler.hpp"
#include "crash_handler/CrashUploader.hpp"
#endif

namespace crashup {

Crashup::Crashup(std::string working_dir, std::string server_address) {
  this->working_dir = working_dir;
  this->server_address = server_address;
  this->executable_directory = QCoreApplication::applicationDirPath().toStdWString();
#if defined(Q_OS_WIN32)
  this->_crashpad_client = nullptr;
#elif defined(Q_OS_LINUX)
  this->_crash_handler = nullptr;
  this->_crash_uploader = nullptr;
#endif
}

void Crashup::init() {
  /* get the breakpad / crashpad handler going -- minidumps written to a
  requested dir throws exception if requested path is inaccessible */
  this->initCrashHandler();

  /* initiates CrashUploader, configured to upload onto server_address */
  /* given to the crashup constructor */
  this->initCrashUploader();
}

void Crashup::setAppName(const std::string &name) { this->app_name = name; }
void Crashup::setAppVersion(const std::string &version) {
  this->app_version = version;
}
void Crashup::setRateLimit(bool throttle) { this->throttle = throttle; }

////////////////////////////////////////////////////////////////////////////

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
  // TODO: This path should be configurable
  std::wstring handler(this->executable_directory + L"\\crashpad_handler.exe");
  std::wstring crashdb;
  std::copy(this->working_dir.begin(), this->working_dir.end(),
            std::back_inserter(crashdb));
  crashdb += L"\\crashdb";

  // additional options for 'crashpad_handler' executable:
  std::vector<std::string> handler_options;
  if (!this->throttle) {
    handler_options.push_back("--no-rate-limit");
  }

  int res = _crashpad_client->StartHandler(
      base::FilePath(handler), base::FilePath(crashdb),
      this->server_address + this->upload_path,
      // data to send with POST requests uploading minidumps:
      std::map<std::string, std::string>{{"ProductName", this->app_name},
                                         {"Version", this->app_version}},
      handler_options, false);
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
// this->_crash_handler->setReportCrashesToSystem(true);
#elif defined(Q_OS_MAC)
  throw TODOException("Crashup::initCrashHandler -- no OS_MAC support")
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
      QString::fromStdString(this->server_address + this->upload_path),
      QString::fromStdString(minidumps_abspath));
#elif defined(Q_OS_MAC)
  throw TODOException("Crashup::initCrashUploader -- no OS_MAC support")
#endif
}

////////////////////////////////////////////////////////////////////////////

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

void Crashup::uploadPendingMinidumps() {
#if defined(Q_OS_LINUX)
  if (_crash_uploader != nullptr)
    _crash_uploader->uploadPendingMinidumps();
  else
#endif
    throw CrashupInitializationException("CrashUploader not initialized.");
}

} // namespace crashup
