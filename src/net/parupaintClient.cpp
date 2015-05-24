

#include "parupaintClient.h"


#include <QDebug>

ParupaintClient::ParupaintClient(const QUrl u, QObject * parent) : QObject(parent), url(u), Connected(false)
{
	connect(&socket, &QWebSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWebSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	socket.open(url);

}
void ParupaintClient::Connect(QUrl u)
{
	if(!u.isEmpty()) url = u;
	if(Connected) this->Disconnect();
	socket.open(url);
}

void ParupaintClient::Disconnect()
{
	if(!Connected) return;
	socket.close();
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
	Connected = true;
	connect(&socket, &QWebSocket::textMessageReceived, this, &ParupaintClient::textReceived);
	emit onMessage("connect", "");
}
void ParupaintClient::onDisconnect()
{
	Connected = false;
}
void ParupaintClient::textReceived(QString text)
{
	if(text.isEmpty()) return;
	const auto list = text.split(" ");
	emit onMessage(list[0], list[1].toUtf8());
}
