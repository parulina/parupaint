
#include "parupaintConnection.h"
#include "parupaintServer.h"
#include "QtWebsocket/QWsSocket.h"

using namespace QtWebsocket;

#include <QDebug>

ParupaintServer::ParupaintServer(quint16 port, QObject * parent) : QObject(parent)
{
	qDebug() << "Starting server at" << port;
	server = new QWsServer(this);
	if(server->listen(QHostAddress::AnyIPv4, port)) {
		connect(server, &QWsServer::newConnection, this, &ParupaintServer::onConnection);
	}
}

ParupaintServer::~ParupaintServer()
{
	server->close();
	qDeleteAll(connections.begin(), connections.end());
}

void ParupaintServer::onConnection()
{
	auto *socket = server->nextPendingConnection();

	connect(socket, SIGNAL(frameReceived(QString)), this, SLOT(textReceived(QString)));
	connect(socket, SIGNAL(disconnected()), this, SLOT(onDisconnection()));
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connections << new ParupaintConnection(socket);
	emit onMessage(connections.first(), "connect", "");
}

void ParupaintServer::onError(QAbstractSocket::SocketError)
{
}

void ParupaintServer::textReceived(QString text)
{
	auto * socket = qobject_cast<QtWebsocket::QWsSocket *>(sender());

	if(text.isEmpty()) return;
	const auto id = text.split(" ")[0];
	const auto arg = text.mid(id.length()+1);
	emit onMessage(GetConnection(socket), id, arg.toUtf8());
}

ParupaintConnection * ParupaintServer::GetConnection(QtWebsocket::QWsSocket* s)
{
	for(auto i = connections.begin(); i != connections.end(); ++i){
		if( (*i)->socket == s ) return (*i);
	}
	return nullptr;
}

void ParupaintServer::onDisconnection()
{
	auto *socket = dynamic_cast<QtWebsocket::QWsSocket *>(sender());
	if(socket) {
		emit onMessage(GetConnection(socket), "disconnect");
		connections.removeOne(GetConnection(socket));
		socket->deleteLater();
	}
}
