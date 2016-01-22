#include "parupaintServer.h"
#include "parupaintConnection.h"
#include "ws/QWsServer.h"

#include <QDebug>

ParupaintServer::ParupaintServer(quint16 port, QObject * parent) : QObject(parent), protective(false)
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
void ParupaintServer::setProtective(bool b)
{
	protective = b;
}
bool ParupaintServer::isProtective()
{
	return protective;
}

void ParupaintServer::onConnection()
{
	QWsSocket *socket = server->nextPendingConnection();

	connect(socket, &QWsSocket::textReceived, this, &ParupaintServer::textReceived);
	connect(socket, &QWsSocket::disconnected, this, &ParupaintServer::onDisconnection);

	// If someone from outside joins...
	if(isProtective() && socket->host() != "localhost"){
		bool has = false;
		// Check if the owner is in the server.
		foreach(ParupaintConnection * con, connections){
			if(con->socket()->host() == "localhost") has = true;
		}
		if(!has){
			socket->abort("Not allowed.");
			qDebug() << "Do not allow socket";
			return;
		}
	}

	ParupaintConnection * con = new ParupaintConnection(socket);

	connections << con;
	message(con, "connect");

}

void ParupaintServer::onDisconnection()
{
	QWsSocket * socket = qobject_cast<QWsSocket* >(sender());
	if(!socket) return;

	ParupaintConnection * con = this->ppConnection(socket);
	if(con){
		connections.removeOne(con);
		message(con, "disconnect");
		delete con;
	}
	socket->deleteLater();
}

void ParupaintServer::textReceived(QString text)
{
	QWsSocket * socket = qobject_cast<QWsSocket* >(sender());
	if(!socket) return;


	if(text.isEmpty()) return;

	ParupaintConnection * con = this->ppConnection(socket);
	if(con){
		const QString id = text.split(" ")[0];
		const QString arg = text.mid(id.length()+1);
		message(con, id, arg.toUtf8());
	}
}

ParupaintConnection * ParupaintServer::ppConnection(QWsSocket* s)
{
	for(auto i = connections.begin(); i != connections.end(); ++i){
		if( (*i)->socket() == s ) return (*i);
	}
	return nullptr;
}

QList<ParupaintConnection*> ParupaintServer::ppConnections()
{
	return connections;
}

int ParupaintServer::ppNumConnections() const
{
	return connections.size();
}
