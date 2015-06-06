
#include <QtNetwork>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>

#include "parupaintVersion.h"
#include "parupaintVersionCheck.h"

const QString url = "https://api.github.com/repos/paruluna/parupaint/releases/latest";

ParupaintVersionCheck::ParupaintVersionCheck()
{
	auto * qnam = new QNetworkAccessManager(this);
	qnam->get(QNetworkRequest(QUrl(url)));
	this->connect(qnam, &QNetworkAccessManager::finished, this, &ParupaintVersionCheck::completed);
}

void ParupaintVersionCheck::completed(QNetworkReply* reply)
{
	if(reply->error()){
		emit Response(false, reply->errorString());
	} else {
		QJsonParseError error;
		auto doc = QJsonDocument::fromJson(reply->readAll(), &error);
		if(error.error == QJsonParseError::NoError){
			QJsonObject main = doc.object();

			QString version = main.value("tag_name").toString();
			QString name = main.value("name").toString();
			QString url = main.value("html_url").toString();

			QString current = "v" + QString(PARUPAINT_VERSION);

			if(version != current){
				QString msg = QString("New update is available! [<a href=\"%1\">%2 (%3)</a>]").arg(url, name, version);
				emit Response(true, msg);
				return;
			}
		}
		emit Response(false, "");
	}
}
