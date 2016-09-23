#include "Crashup.hpp"
#include "exceptions.hpp"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QString>
#include <cstdlib>
#include <iostream>

#include <codecvt>

#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
#include "client/crash_report_database.h"
#include "client/crashpad_client.h"
#include "client/crashpad_info.h"
#include "client/settings.h"
#include "base/files/file_path.h"
#endif

namespace {
template <typename StringType> StringType string_cast(const std::string &s);

template <> std::string string_cast(const std::string &s) { return s; }

template <> std::wstring string_cast(const std::string &s) {
  static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  return converter.from_bytes(s);
}
}

namespace crashup {

Crashup::Crashup(std::string app_name, std::string app_version,
                 std::string data_directory, std::string upload_url)
    : app_name(app_name), app_version(app_version),
      data_directory(data_directory), upload_url(upload_url) {
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
  // Get executable directory to find crashpad_handler then.
  this->executable_directory =
      QCoreApplication::applicationDirPath().toStdString();

  this->_crashpad_client = nullptr;
#endif
}

void Crashup::init() {
  /* get the breakpad / crashpad handler going -- minidumps written to a
  requested dir throws exception if requested path is inaccessible */
  this->initCrashHandler();

  /* initiates CrashUploader, configured to upload onto upload_url */
  /* given to the crashup constructor */
  this->initCrashUploader();
}

void Crashup::setRateLimit(bool throttle) { this->throttle = throttle; }

////////////////////////////////////////////////////////////////////////////

std::string Crashup::makeInternalDirPath(const std::string &dirpath) {
  QString final_dir_path = QDir(QString::fromStdString(this->data_directory))
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
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
  // initialize crash handler
  this->_crashpad_client = new crashpad::CrashpadClient();

#if defined(Q_OS_WIN32)
  std::string handler(this->executable_directory + "\\crashpad_handler.exe");
  std::string crashdb(this->data_directory + "\\crashdb");
#else
  std::string handler(this->executable_directory + "/crashpad_handler");
  std::string crashdb(this->data_directory + "/crashdb");
#endif

  // additional options for 'crashpad_handler' executable:
  std::vector<std::string> handler_options;
  if (!this->throttle) {
    handler_options.push_back("--no-rate-limit");
  }

  int res = _crashpad_client->StartHandler(
      base::FilePath(string_cast<base::FilePath::StringType>(handler)),
      base::FilePath(string_cast<base::FilePath::StringType>(crashdb)),
      this->upload_url,
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
#else
  throw TODOException("Crashup::initCrashHandler");
#endif
}

void Crashup::initCrashUploader() {
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
#if defined(Q_OS_WIN32)
  std::string crashdb(this->data_directory + "\\crashdb");
#else
  std::string crashdb(this->data_directory + "/crashdb");
#endif

  // turn on minidump uploads in crashdb settings
  int res =
      crashpad::CrashReportDatabase::Initialize(
          base::FilePath(string_cast<base::FilePath::StringType>(crashdb)))
          ->GetSettings()
          ->SetUploadsEnabled(true);
  if (!res) {
    throw CrashupInitializationException(
        "crashpad::CrashReportDatabase::Settings::SetUploadsEnabled failed");
  }
#else
  throw TODOException("Crashup::initCrashUploader");
#endif
}

} // namespace crashup
