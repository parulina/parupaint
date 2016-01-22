#include "server_bundled.h"

#include <QJsonObject>

#include "../net/parupaintConnection.h"

ParupaintBundledServer::ParupaintBundledServer(quint16 port, QObject * parent) :
	ParupaintServerInstance(port, parent)
{
	connect(this, &ParupaintServerInstance::onConnect, this, &ParupaintBundledServer::onPlayerConnect);
	connect(this, &ParupaintServerInstance::onDisconnect, this, &ParupaintBundledServer::onPlayerDisconnect);
	connect(this, &ParupaintServerInstance::onJoin, this, &ParupaintBundledServer::onPlayerJoin);
	connect(this, &ParupaintServerInstance::onLeave, this, &ParupaintBundledServer::onPlayerLeave);
}

void ParupaintBundledServer::onPlayerConnect(ParupaintConnection * con)
{
	if(!con->isLocal()){
		this->sendChat("Someone connected.", con);

		QJsonObject obj;
		obj["message"] = QString("There are %1 painters in this room.").arg(this->numPainters());
		con->send("chat", obj);
	}
}

void ParupaintBundledServer::onPlayerDisconnect(ParupaintConnection * con)
{
	if(!con->isLocal()){
		this->sendChat(QString("%1 left.").arg(con->name()), con);
	}
}

void ParupaintBundledServer::onPlayerJoin(ParupaintConnection * con)
{
	if(!con->isLocal()){
		this->sendChat(QString("%1 started drawing.").arg(con->name()));
	}
}

void ParupaintBundledServer::onPlayerLeave(ParupaintConnection * con)
{
	if(!con->isLocal()){
		this->sendChat(QString("%1 stopped drawing.").arg(con->name()));
	}
}

void ParupaintBundledServer::message(ParupaintConnection * con, const QString & id, const QByteArray & array)
{
	ParupaintServerInstance::message(con, id, array);
}
