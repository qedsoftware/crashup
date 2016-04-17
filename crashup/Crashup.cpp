#include "Crashup.hpp"
#include <QDir>
#include <QString>
#include <QDebug>

namespace crashup {

const std::string Crashup::report_minidumps_relative_path = "minidumps";

Crashup::Crashup(std::string working_dir, std::string server_address)
    : _stats(working_dir, server_address) {
  this->report_minidumps_dirpath = "";
  this->working_dir = working_dir;
  this->server_address = server_address;
  this->_crashHandler = nullptr;
  this->_crashUploader = nullptr;
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

  std::string report_minidumps_absolute_dirpath =
      Crashup::report_minidumps_relative_path;
  if (checkRelativeDirpath(report_minidumps_absolute_dirpath)) {
    /* report_minidumps_absolute_dirpath is a valid absolute path now */
    this->report_minidumps_dirpath = report_minidumps_absolute_dirpath;
  } else {
    throw CrashupInitMinidumpDirpathException();
  }

//#if defined(Q_OS_WIN32)
//  throw TODOException("Crashup::initCrashHandler -- no OS_WIN support")
//#elif defined(Q_OS_LINUX)
#if 1
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
  if (_crashHandler != nullptr)
    _crashHandler->writeMinidump();
  else
    throw CrashupInitializationException("CrashHandler not initialized.");
}

/* initializing Crash Uploader does not require initizing Crash Handler */
/* first (or even at all)                                               */
void Crashup::initCrashUploader(const std::string &product_name,
                                const std::string &product_version) {
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
}

void Crashup::uploadPendingMinidumps() {
  if (_crashUploader != nullptr)
    _crashUploader->uploadPendingMinidumps();
  else
    throw CrashupInitializationException("CrashUploader not initialized.");
}

}; // namespace crashup
