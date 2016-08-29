#pragma once
#include <QCoreApplication>
#include <QDebug>
#include <QObject>
#include <QThread>
#include <QString>
#include <QDir>
#include <string>
#include <map>
#include "../exceptions.hpp"

namespace crash_handling {

/**
 * Private things to upload minidumps.
 */
class CrashUploaderPrivate : public QObject {
  Q_OBJECT
public:
  CrashUploaderPrivate(const QString &product_name,
                       const QString &product_version,
                       const QString &crash_server);
public slots:
  void uploadMinidumpPrivate(const QString &minidump_filename);
signals:
  void resultReady(bool result, const QString &minidump_filename,
                   const QString &http_response_body);

private:
  QString product_name, product_version;
  QString crash_server;
};

/* ************* */
/* main interest */
/* ************* */

/**
 * This class controls uploading minidumps on Linux.
 */
class CrashUploader : public QObject {
  Q_OBJECT
  QThread uploadingThread;

public:
  CrashUploader(const QString &product_name, const QString &product_version,
                const QString &crash_server,
                const QString &saved_minidumps_dirpath);
  ~CrashUploader();

  /**
   * Uploads minidumps not registered in `.minidumps.json`
   */
  void uploadPendingMinidumps();

public slots:
  void uploadFinished(bool result, const QString &minidump_filename,
                      const QString &http_response_body);

signals:
  void uploadMinidump(const QString &minidump_filename);

private:
  QString minidump_metadata_filename = ".minidumps.json";
  QString product_name, product_version;
  QString crash_server;
  QDir saved_minidumps_dir;

  /**
   * Registers minidump file in `.minidumps.json` database.
   *
   * @param local_minidump_filename: Minidump file in local filesystem.
   * @param remote_minidump_filename: Crash id on remote server.
   */
  void add_minidump_to_metadata(const QString &local_minidump_filename,
                                const QString &remote_minidump_id);

  /**
   * Returns database with registered minidumps
   * (i.e. contents of `.minidumps.json`)
   */
  QJsonArray get_minidumps_metadata();
  Q_DISABLE_COPY(CrashUploader)
};
};
