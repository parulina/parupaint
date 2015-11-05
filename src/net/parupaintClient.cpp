#include "parupaintClient.h"

#include <QStringBuilder>
#include <QDebug>

ParupaintClient::ParupaintClient(QObject * parent) : QObject(parent), Connected(false), SwitchHost(false)
{
	connect(&socket, &QWsSocket::connected, this, &ParupaintClient::onConnect);
	connect(&socket, &QWsSocket::disconnected, this, &ParupaintClient::onDisconnect);
	connect(&socket, &QWsSocket::textReceived, this, &ParupaintClient::textReceived);
	connect(&socket, &QWsSocket::stateChanged, this, &ParupaintClient::onSocketStateChanged);
	connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
}

void ParupaintClient::Connect(QString u)
{
	this->Connect(u.section(":", 0, 0), u.section(":", 1, 1).toInt());
}

void ParupaintClient::Connect(QString u, quint16 p)
{
	if(p == 0) p = u.section(":", 1, 1).toInt();
	if(p == 0) p = 1108;

	if(u.contains("//")) u = u.section("//", 1);
	u = u.section(":", 0, 0);

	host = "ws://" + u;
	port = p;

	qDebug() << "Connecting to" << host << port;

	if(socket.state() != QAbstractSocket::UnconnectedState){
		SwitchHost = true;
		return this->Disconnect("SwitchHost");
	} else {
		socket.connectToHost(host, p);
	}
}

void ParupaintClient::Disconnect(QString reason)
{
	if(socket.state() != QAbstractSocket::UnconnectedState) {
		if(socket.state() == QAbstractSocket::ConnectedState){
			socket.disconnectFromHost(reason);
		} else {
			socket.abort();
		}
	}
}

void ParupaintClient::onSocketStateChanged(QAbstractSocket::SocketState)
{
}

void ParupaintClient::onError(QAbstractSocket::SocketError)
{
	emit onMessage("error", socket.errorString().toUtf8());
}
qint64 ParupaintClient::send(const QString data)
{
	if(socket.state() != QAbstractSocket::ConnectedState) return 0;
	return socket.sendTextMessage(data);
}
qint64 ParupaintClient::send(const QString id, const QString data)
{
	return this->send(id % " " % data);
}

void ParupaintClient::onConnect()
{
	emit onMessage("connect");
}
void ParupaintClient::onDisconnect()
{
	emit onMessage("disconnect");

	if(SwitchHost){
		SwitchHost = false;
		qDebug() << "Disconnected, connecting again to " << host << port;
		this->Connect(host, port);
	}
}
void ParupaintClient::textReceived(QString text)
{
	if(text.isEmpty()) return;
	const auto id = text.split(' ')[0];
	const auto arg = text.mid(id.length()+1);
	emit onMessage(id, arg.toUtf8());
}
