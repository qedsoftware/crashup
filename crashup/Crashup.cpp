#include "Crashup.hpp"
#include <QDir>
#include <QString>
#include <QDebug>

namespace crashup {

Crashup::Crashup(std::string working_dir, std::string server_address)
    : _stats(working_dir, server_address) {
  this->working_dir = working_dir;
  this->server_address = server_address;
  this->_crashHandler = nullptr;
}

Stats &Crashup::stats() { return _stats; }

void Crashup::initCrashHandler(
    const std::string &report_minidumps_relative_dirpath) {

  QString final_dir_path =
      QDir(QString::fromStdString(working_dir))
          .filePath(QString::fromStdString(report_minidumps_relative_dirpath));
  QDir final_dir = QDir(final_dir_path);
  if (!final_dir.exists()) {
    // create the dir in case it doesn't exist
    QDir().mkpath(final_dir_path);
  }

  if (!(QFileInfo(final_dir_path).isDir() &&
        QFileInfo(final_dir_path).isWritable())) {
    // if the requested dir still doesn't exist or there is no permittion to
    // write in it -- throw exception
    throw CrashupInitMinidumpDirpathException();
  }

  std::string report_minidumps_absolute_dirpath =
      final_dir.absolutePath().toUtf8().constData();

#if defined(Q_OS_WIN32)
  throw TODOException("Crashup::initCrashHandler -- no OS_WIN support")
#elif defined(Q_OS_LINUX)
  this->_crashHandler = crashhandler::CrashHandler::instance();
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
    throw CrashupWriteMinidumpException("CrashHandler not initialized.");
}

}; // namespace crashup
