#include "parupaintClient.h"

#include <QDebug>

using namespace QtWebsocket;

ParupaintClient::ParupaintClient(QObject * parent) : QObject(parent), Connected(false), SwitchHost(false)
{
	connect(&socket, &QWsSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QAbstractSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, SIGNAL(frameReceived(QString)), this, SLOT(textReceived(QString)));
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connect(&socket, &QAbstractSocket::stateChanged, this, &ParupaintClient::onSocketStateChanged);
}

void ParupaintClient::Connect(QString u, quint16 p)
{
	QString prefix = "ws://";
	if(u.indexOf(prefix) != 0){
		u = prefix + u.section("/", -1);
	}

	QString pp = u.section(":", -1);
	if(pp.length()){
		port = pp.toInt();
		u.resize(u.length() - (1+pp.length()));
	}
	if(port == 0) port = 1108;
	if(p != 0) port = p;

	if(!u.isEmpty()) host = u;
	qDebug() << "Connecting to" << host << port;

	if(Connected){
		SwitchHost = true;
		return this->Disconnect();
	} else {
		socket.connectToHost(host, port);
	}
}

void ParupaintClient::Disconnect()
{
	if(!Connected) return;
	// TODO FIXME
	// Why doesn't disconnectFromHost work?? to reconnect
	socket.abort();
}

void ParupaintClient::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
	if(socketState == QAbstractSocket::UnconnectedState){
	}
}

void ParupaintClient::onError(QAbstractSocket::SocketError)
{
	emit onMessage("error", socket.errorString().toUtf8());
}
void ParupaintClient::send(QString id, QString data)
{
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
	emit onMessage("disconnect", "");

	if(SwitchHost){
		SwitchHost = false;
		socket.connectToHost(host, port);
	}
}
void ParupaintClient::textReceived(QString text)
{
	if(text.isEmpty()) return;
	const auto id = text.split(' ')[0];
	const auto arg = text.mid(id.length()+1);
	emit onMessage(id, arg.toUtf8());
}
