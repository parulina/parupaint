

#include "parupaintClient.h"


#include <QDebug>

ParupaintClient::ParupaintClient(const QUrl u, QObject * parent) : QObject(parent), url(u)
{
	connect(&socket, &QWebSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWebSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	socket.open(url);

}
void ParupaintClient::onError(QAbstractSocket::SocketError )
{
}
void ParupaintClient::send(QString id, QString data)
{
	socket.sendTextMessage(id + " " + data);
}

void ParupaintClient::onConnect()
{
	connect(&socket, &QWebSocket::textMessageReceived, this, &ParupaintClient::textReceived);
	emit onMessage("connect", "");
}
void ParupaintClient::onDisconnect()
{

}
void ParupaintClient::textReceived(QString text)
{
	QString id = text.split(" ")[0];
	const QByteArray json = text.split(" ")[1].toUtf8();

	if(!id.isEmpty()) {
		emit onMessage(id, json);
	}
}
