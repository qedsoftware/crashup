#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <vector>

#include <QFile>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

class Downloader : public QObject {
  Q_OBJECT

public:
  void downloadFile();
  Downloader(QString downloadDirectory, QUrl url);

  // first is number of arguments, then retry periods given in miliseconds
  template <typename... T> void setRetryPeriods(T... args);

private slots:
  void httpReadyRead();
  void httpFinished();
  void startRequest(const QUrl &requestedUrl);

signals:
  void downloadFinished(QString filePath);

private:
  QFile *openFileForWrite(const QString &fileName);
  template <typename T> void addRetryPeriods(T arg);
  template <typename U, typename... T> void addRetryPeriods(U head, T... tail);

  QUrl url;
  QNetworkAccessManager qnam;
  QNetworkReply *reply;
  QFile *file;
  QString downloadDirectory;
  bool doResume;

  std::vector<int> retryPeriods;
  int currentRetry;
};

#endif
