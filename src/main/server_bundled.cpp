#include "server_bundled.h"

#include <QJsonObject>

#include "../net/parupaintConnection.h"

ParupaintBundledServer::ParupaintBundledServer(quint16 port, QObject * parent) :
	ParupaintServerInstance(port, parent)
{
}

void ParupaintBundledServer::message(ParupaintConnection * con, const QString & id, const QByteArray & array)
{
	if(id == "join"){
		if(!con->isLocal()){
			QJsonObject obj;
			obj["message"] = con->name() + " joined.";
			this->sendAll("chat", obj);

			if(!this->password().isEmpty()){
				obj["message"] = "Participating is passworded, use /join <password> to join";
				con->send("chat", obj);
			}
		}
	}
	if(id == "leave"){
		if(!con->isLocal()){
			QJsonObject obj;
			obj["message"] = con->name() + " left.";
			this->sendAll("chat", obj);
		}
	}
	ParupaintServerInstance::message(con, id, array);
}
