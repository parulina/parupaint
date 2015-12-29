#include "server_bundled.h"

#include <QJsonObject>

#include "../net/parupaintConnection.h"

ParupaintBundledServer::ParupaintBundledServer(quint16 port, QObject * parent) :
	ParupaintServerInstance(port, parent)
{
	connect(this, &ParupaintServerInstance::onJoin, this, &ParupaintBundledServer::onPlayerJoin);
	connect(this, &ParupaintServerInstance::onLeave, this, &ParupaintBundledServer::onPlayerLeave);
}

void ParupaintBundledServer::onPlayerJoin(ParupaintConnection * con)
{
	if(!con->isLocal()){
		QJsonObject obj;
		obj["message"] = con->name() + " joined.";
		this->sendAll("chat", obj);
	}
}
void ParupaintBundledServer::onPlayerLeave(ParupaintConnection * con)
{
	if(!con->isLocal()){
		QJsonObject obj;
		obj["message"] = con->name() + " left.";
		this->sendAll("chat", obj);
	}
}

void ParupaintBundledServer::message(ParupaintConnection * con, const QString & id, const QByteArray & array)
{
	ParupaintServerInstance::message(con, id, array);
}
