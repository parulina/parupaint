
#include <QtWebSockets/QWebSocketServer>
#include <QtWebSockets/QWebSocket>
#include "parupaintConnection.h"
#include "parupaintServer.h"



ParupaintServer::ParupaintServer(quint16 port, QObject * parent) : QObject(parent)
{
	server = new QWebSocketServer("Parupaint server", QWebSocketServer::NonSecureMode, this);
	if(server->listen(QHostAddress::AnyIPv4, port)) {
		connect(server, &QWebSocketServer::newConnection, this, &ParupaintServer::onConnection);
		connect(server, &QWebSocketServer::closed, this, &ParupaintServer::onDisconnection);
	}
}

ParupaintServer::~ParupaintServer()
{
	server->close();
	qDeleteAll(connections.begin(), connections.end());
}

void ParupaintServer::onConnection()
{
	QWebSocket *socket = server->nextPendingConnection();

	qDebug() << "Connection" << socket;

	connect(socket, &QWebSocket::textMessageReceived, this, &ParupaintServer::textReceived);
	connect(socket, &QWebSocket::disconnected, this, &ParupaintServer::onDisconnection);
	connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
	connections << new ParupaintConnection(socket);
	emit onMessage(connections.first(), "connect", "");
}

void ParupaintServer::onError(QAbstractSocket::SocketError error)
{
	qDebug() << "Socket error!" << error;
}

void ParupaintServer::textReceived(QString text)
{
	QWebSocket * socket = qobject_cast<QWebSocket *>(sender());

	if(text.isEmpty()) return;
	const auto list = text.split(" ");
	emit onMessage(GetConnection(socket), list[0], list[1].toUtf8());
}

ParupaintConnection * ParupaintServer::GetConnection(QWebSocket* s)
{
	for(auto i = connections.begin(); i != connections.end(); ++i){
		if( (*i)->socket == s ) return (*i);
	}
	return nullptr;
}

void ParupaintServer::onDisconnection()
{
	QWebSocket *socket = dynamic_cast<QWebSocket *>(sender());
	if(socket) {
		emit onMessage(GetConnection(socket), "disconnect");
		connections.removeOne(GetConnection(socket));
		socket->deleteLater();
	}
}
