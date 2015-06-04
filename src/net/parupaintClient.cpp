

#include "parupaintClient.h"


#include <QDebug>

ParupaintClient::ParupaintClient(const QString u, QObject * parent) : QObject(parent), url(u), Connected(false)
{
	connect(&socket, &QWebSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWebSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, &QWebSocket::textMessageReceived, this, &ParupaintClient::textReceived);

	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	socket.open(url);

}
void ParupaintClient::Connect(QString u)
{
	QString prefix = "ws://";
	if(u.indexOf(prefix) != 0){
		u = prefix + u.section("/", -1);
	}

	if(!u.isEmpty()) url = QUrl(u);
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
	socket.flush();
}

void ParupaintClient::onConnect()
{
	Connected = true;
	emit onMessage("connect", "");
}
void ParupaintClient::onDisconnect()
{
	Connected = false;
}
void ParupaintClient::textReceived(QString text)
{
	if(text.isEmpty()) return;
	const auto id = text.split(' ')[0];
	const auto arg = text.mid(id.length()+1);
	emit onMessage(id, arg.toUtf8());
}
