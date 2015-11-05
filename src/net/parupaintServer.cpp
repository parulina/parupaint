#include "parupaintServer.h"
#include "parupaintConnection.h"
#include "ws/QWsServer.h"

#include <QDebug>

ParupaintServer::ParupaintServer(quint16 port, QObject * parent) : QObject(parent)
{
	qDebug() << "Starting server at" << port;
	server = new QWsServer(this);
	if(server->listen(QHostAddress::Any, port)) {
		connect(server, &QWsServer::newConnection, this, &ParupaintServer::onConnection);
		//connect(server, &QWsServer::closed, this, &ParupaintServer::closed);
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

	connect(socket, &QWsSocket::textReceived, this, &ParupaintServer::textReceived);
	connect(socket, &QWsSocket::disconnected, this, &ParupaintServer::onDisconnection);

	connections << new ParupaintConnection(socket);
	emit onMessage(connections.first(), "connect");
}

void ParupaintServer::onDisconnection()
{
	auto *socket = dynamic_cast<QWsSocket* >(sender());
	if(socket) {
		emit onMessage(GetConnection(socket), "disconnect");
		connections.removeOne(GetConnection(socket));
		socket->deleteLater();
	}
}

void ParupaintServer::textReceived(QString text)
{
	auto * socket = qobject_cast<QWsSocket* >(sender());

	if(text.isEmpty()) return;
	const auto id = text.split(" ")[0];
	const auto arg = text.mid(id.length()+1);
	emit onMessage(GetConnection(socket), id, arg.toUtf8());
}

ParupaintConnection * ParupaintServer::GetConnection(QWsSocket* s)
{
	for(auto i = connections.begin(); i != connections.end(); ++i){
		if( (*i)->getSocket() == s ) return (*i);
	}
	return nullptr;
}
