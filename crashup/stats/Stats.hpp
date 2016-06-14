#include <string>
#include <map>
#include <QUrl>
#include <QUrlQuery>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>

class Stats {
  std::string working_dir, server_address;
  QNetworkAccessManager qnam;

public:
  Stats(std::string working_dir, std::string server_address);
  void send(const std::map<std::string, std::string> &data) {

    // connect(qnam, SIGNAL(finished(QNetworkReply*)), this,
    // SLOT(handleNetworkData(QNetworkReply*)));
    // connect(qnam,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this,
    // SLOT(handleSSLErrors(QNetworkReply*)));

    QUrl serviceURL((server_address + "/api/upload_usage_report/").c_str());
    QNetworkRequest request(serviceURL);

    request.setRawHeader("User-Agent", "desktop-crashup");
    request.setRawHeader("X-Custom-User-Agent", "desktop-crashup");
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/x-www-form-urlencoded");
    QUrlQuery postData;
    for (const auto &x : data) {
      postData.addQueryItem(x.first.c_str(), x.second.c_str());
    }
    QByteArray postDataEncoded = postData.toString(QUrl::FullyEncoded).toUtf8();

    request.setHeader(QNetworkRequest::ContentLengthHeader,
                      postDataEncoded.size());

    QNetworkReply *reply = qnam.post(request, postDataEncoded);
  }
};
