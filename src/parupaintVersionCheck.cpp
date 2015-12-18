#include "parupaintVersionCheck.h"
#include "parupaintVersion.h"

#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

const QString url = "https://api.github.com/repos/parulina/parupaint/releases/latest";

ParupaintVersionCheck::ParupaintVersionCheck(QObject * parent) : QObject(parent)
{
	QNetworkAccessManager * qnam = new QNetworkAccessManager(this);
	qDebug() << "Check update" << url;
	this->connect(qnam, &QNetworkAccessManager::finished, this, &ParupaintVersionCheck::completed);
	qnam->get(QNetworkRequest(QUrl(url)));
}

void ParupaintVersionCheck::completed(QNetworkReply* reply)
{
	if(!reply->error()){
		QJsonParseError error;
		auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
		if(error.error == QJsonParseError::NoError){
			QJsonObject main = doc.object();

			QString version = main.value("tag_name").toString();
			QString name = main.value("name").toString();
			QString url = main.value("html_url").toString();

			QString current = "v" + QString(PARUPAINT_VERSION);

			bool k;
			qreal real_cur = current.replace(QRegularExpression("[^0-9\\.]"), "").toDouble(&k);
			qreal real_ver = version.replace(QRegularExpression("[^0-9\\.]"), "").toDouble(&k);

			if(k && real_ver > real_cur){
				QString msg = QString("New update is available! [<a href=\"%1\">%2 (%3)</a>]").arg(url, name, version);
				emit updateResponse(msg, true);
				return;
			}
		}
		emit updateResponse("", false);
	}
}
