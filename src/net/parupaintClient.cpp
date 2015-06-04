

#include "parupaintClient.h"


#include <QDebug>

using namespace QtWebsocket;

ParupaintClient::ParupaintClient(QObject * parent) : QObject(parent), Connected(false)
{
	connect(&socket, &QWsSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWsSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, SIGNAL(frameReceived(QString)), this, SLOT(textReceived(QString)));
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void ParupaintClient::Connect(QString u, quint16 p)
{
	QString prefix = "ws://";
	if(u.indexOf(prefix) != 0){
		u = prefix + u.section("/", -1);
	}
	QString pp = u.section(":", -1);
	quint16 port = 0;

	if(pp.length()){
		port = pp.toInt();
		u.resize(u.length() - (1+pp.length()));
	}
	if(port == 0) port = 1108;
	if(p != 0) port = p;

	if(!u.isEmpty()) url = QUrl(u);
	if(Connected) this->Disconnect();
	qDebug() << u << port;
	socket.connectToHost(u, port);
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
