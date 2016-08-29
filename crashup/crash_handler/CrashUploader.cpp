#include "CrashUploader.hpp"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#if defined(Q_OS_LINUX)
#include "libcurl_wrapper.h"
#include "google_crashdump_uploader.h"
#endif

namespace crash_handling {

/* ******************** */
/* CrashUploaderPrivate */
/* ******************** */

CrashUploaderPrivate::CrashUploaderPrivate(const QString &product_name,
                                           const QString &product_version,
                                           const QString &crash_server)
    : product_name(product_name), product_version(product_version),
      crash_server(crash_server) {}

void CrashUploaderPrivate::uploadMinidumpPrivate(
    const QString &minidump_filename) {
#if defined(Q_OS_LINUX)
  google_breakpad::LibcurlWrapper *http_layer =
      new google_breakpad::LibcurlWrapper();
  google_breakpad::GoogleCrashdumpUploader uploader(
      this->product_name.toUtf8().constData(),    // product
      this->product_version.toUtf8().constData(), // version
      "123",                                      // guid (ID is required)
      "",                                         // ptime
      "",                                         // ctime
      "",                                         // email
      "",                                         // comments
      minidump_filename.toUtf8().constData(),     // minidump path
      crash_server.toUtf8().constData(),          // crash_server
      "",                                         // proxy_host
      "",                                         // proxy_userpassword
      http_layer);
  std::string http_response_body;
  if (uploader.Upload(nullptr, nullptr, &http_response_body)) {
    qDebug() << "Successful upload of minidump " << minidump_filename;
    emit resultReady(true, minidump_filename,
                     QString::fromStdString(http_response_body));
  } else {
    qDebug() << "Failure uploading minidump " << minidump_filename;
    emit resultReady(false, minidump_filename,
                     QString::fromStdString(http_response_body));
  }
#endif
}

/* ******************** */
/*    CrashUploader     */
/* ******************** */

CrashUploader::CrashUploader(const QString &product_name,
                             const QString &product_version,
                             const QString &crash_server,
                             const QString &saved_minidumps_dirpath)
    : product_name(product_name), product_version(product_version),
      crash_server(crash_server), saved_minidumps_dir(saved_minidumps_dirpath) {
  CrashUploaderPrivate *privCrashUploader =
      new CrashUploaderPrivate(product_name, product_version, crash_server);
  privCrashUploader->moveToThread(&uploadingThread);
  connect(&uploadingThread, &QThread::finished, privCrashUploader,
          &QObject::deleteLater);
  connect(this, &CrashUploader::uploadMinidump, privCrashUploader,
          &CrashUploaderPrivate::uploadMinidumpPrivate);
  connect(privCrashUploader, &CrashUploaderPrivate::resultReady, this,
          &CrashUploader::uploadFinished);
  uploadingThread.start();
}

CrashUploader::~CrashUploader() {
  uploadingThread.quit();
  uploadingThread.wait();
}

QJsonArray CrashUploader::get_minidumps_metadata() {
  QString json_file_path =
      this->saved_minidumps_dir.filePath(this->minidump_metadata_filename);
  QFile json_file(json_file_path);
  json_file.open(QFile::ReadOnly);
  QString file_content = json_file.readAll();
  QJsonDocument qjson_doc = QJsonDocument::fromJson(file_content.toUtf8());
  QJsonArray qjson_array = qjson_doc.array();
  json_file.close();
  return qjson_array;
}

void CrashUploader::add_minidump_to_metadata(
    const QString &local_minidump_filename,
    const QString &remote_minidump_filename) {
  QString json_file_path =
      this->saved_minidumps_dir.filePath(this->minidump_metadata_filename);
  QFile json_file(json_file_path);
  json_file.open(QFile::ReadWrite);
  QString file_content = json_file.readAll();
  QJsonDocument qjson_doc = QJsonDocument::fromJson(file_content.toUtf8());
  QJsonArray qjson_array = qjson_doc.array();

  QJsonObject qjson_obj_1{{"local_filename", local_minidump_filename},
                          {"remote_filename", remote_minidump_filename}};
  qjson_array << qjson_obj_1;
  json_file.resize(0);
  json_file.write(QJsonDocument(qjson_array).toJson(QJsonDocument::Indented));
  json_file.close();
  return;
}

void CrashUploader::uploadFinished(bool result,
                                   const QString &minidump_filename,
                                   const QString &http_response_body) {
  qDebug() << "HTTP response : " << http_response_body;
  if (result) {
    QRegExp rx_full("CrashID=.{2}-.{8}-.{4}-.{4}-.{4}-.{12}\\n");
    if (rx_full.indexIn(http_response_body) == -1) {
      throw UnexpectedHttpResponseException();
    }
    QString remote_relative_path = "";
    QRegExp rx("CrashID=.{2}-(.{2})(.{2}).*\\n");
    if (rx.indexIn(http_response_body) != -1) {
      remote_relative_path += rx.cap(1);
      remote_relative_path += "/";
      remote_relative_path += rx.cap(2);
      remote_relative_path += "/";
    }
    QRegExp rx_filename("CrashID=.{2}-((.*))\\n");
    if (rx_filename.indexIn(http_response_body) != -1) {
      remote_relative_path += rx_filename.cap(1);
    }
    add_minidump_to_metadata(minidump_filename, remote_relative_path);
  } else {
    // possibly doing something else in case of failure, some retry procedure or
    // something
    throw TODOException("CrashUploader::uploadFinished -- failed upload. TODO "
                        "retry procedure.");
  }
  return;
}

void CrashUploader::uploadPendingMinidumps() {
  QDir q_minidumps_dir = this->saved_minidumps_dir;
  q_minidumps_dir.setNameFilters(QStringList() << "*.dmp");
  QFileInfoList fileList = q_minidumps_dir.entryInfoList();
  QJsonArray metadata = this->get_minidumps_metadata();
  for (QFileInfoList::iterator it = fileList.begin(); it != fileList.end();
       ++it) {
    QString path = (*it).absoluteFilePath();
    bool uploaded = false;
    for (QJsonArray::iterator it = metadata.begin(); it != metadata.end();
         it++) {
      if ((*it).toObject().value("local_filename").toString() == path) {
        uploaded = true;
        break;
      }
    }
#if defined(Q_OS_LINUX)
    if (!uploaded) {
      emit uploadMinidump(path);
    }
#endif
  }
}

}; // namespace crash_handling
