

#include "parupaintClient.h"


#include <QDebug>

using namespace QtWebsocket;

ParupaintClient::ParupaintClient(const QString u, QObject * parent) : QObject(parent), url(u), Connected(false)
{
	connect(&socket, &QWsSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWsSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, SIGNAL(frameReceived(QString)), this, SLOT(textReceived(QString)));

	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	socket.connectToHost(u, 1108);

}
void ParupaintClient::Connect(QString u)
{
	QString prefix = "ws://";
	if(u.indexOf(prefix) != 0){
		u = prefix + u.section("/", -1);
	}

	if(!u.isEmpty()) url = QUrl(u);
	if(Connected) this->Disconnect();
	socket.connectToHost(u, 1108);
}

void ParupaintClient::Disconnect()
{
	if(!Connected) return;
	socket.QAbstractSocket::disconnectFromHost();
}


void ParupaintClient::onError(QAbstractSocket::SocketError )
{
}
void ParupaintClient::send(QString id, QString data)
{
	//socket.sendTextMessage(id + " " + data);
	//socket.flush();
	socket.write(id + " " + data);
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
