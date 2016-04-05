#include "Downloader.hpp"
#include "../exceptions.hpp"

#include <QTimer>

Downloader::Downloader(QString downloadDirectory, QUrl url)
    : doResume(false), currentRetry(-1) {
  this->downloadDirectory = downloadDirectory;
  this->url = url;
  this->retryPeriods.push_back(5000);
}

void Downloader::downloadFile() {
  QString fileName = downloadDirectory + "/" + url.fileName() + ".part";
  file = openFileForWrite(fileName);
  startRequest(url);
}

void Downloader::httpReadyRead() {
  if (file) {
    file->write(reply->readAll());
  }

  else {
    throw NoOpenedFileException();
  }
}

void Downloader::startRequest(const QUrl &requestedUrl) {
  if (doResume) {
    QNetworkRequest req(requestedUrl);
    file->flush();
    QByteArray rangeHeaderValue =
        "bytes=" + QByteArray::number(file->size()) + "-";
    req.setRawHeader("Range", rangeHeaderValue);
    reply = qnam.get(req);
  } else {
    reply = qnam.get(QNetworkRequest(requestedUrl));
  }

  connect(reply, &QNetworkReply::finished, this, &Downloader::httpFinished);
  connect(reply, &QIODevice::readyRead, this, &Downloader::httpReadyRead);
}

void Downloader::httpFinished() {
  if (reply->error()) {
    doResume = true;
    if (currentRetry < int(retryPeriods.size()) - 1)
      currentRetry++;
    QTimer::singleShot(retryPeriods[currentRetry], this, SLOT(downloadFile()));
  }

  else {
    if (file) {
      QString fname = file->fileName();
      fname.resize(fname.size() - 5); // removes '.part' from the end
      file->rename(fname);
      emit downloadFinished(file->fileName());

      file->close();
      delete file;
      file = Q_NULLPTR;
    }
  }

  reply->deleteLater();
  reply = Q_NULLPTR;
}

QFile *Downloader::openFileForWrite(const QString &fileName) {
  QScopedPointer<QFile> file(new QFile(fileName));
  file->open(QIODevice::WriteOnly);
  return file.take();
}

template <typename T> void Downloader::addRetryPeriods(T tail) {
  retryPeriods.push_back(tail);
}

template <typename U, typename... T>
void Downloader::addRetryPeriods(U head, T... tail) {
  retryPeriods.push_back(head);
  addRetryPeriods(tail...);
}

template <typename... T> void Downloader::setRetryPeriods(T... args) {
  retryPeriods.clear();
  addRetryPeriods(args...);
}
