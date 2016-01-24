#include "parupaintConnection.h"

#include <QStringBuilder>
#include <QJsonDocument>
#include <QDebug>

ParupaintConnection::ParupaintConnection(QWsSocket * s) :
	connection_socket(s),
	connection_id(0),
	autojoin_flag(false)
{
}

qint64 ParupaintConnection::send(const QString id, const QJsonObject &obj)
{
	return this->send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

qint64 ParupaintConnection::send(const QString id, const QString msg)
{
	if(!this->connection_socket) return 0;
	return connection_socket->sendTextMessage(id % " " % msg);
}

void ParupaintConnection::setId(int id)
{
	this->connection_id = id;
}
int ParupaintConnection::id() const
{
	return this->connection_id;
}

void ParupaintConnection::setName(const QString & name)
{
	this->connection_name = name;
}
QString ParupaintConnection::name()
{
	return this->connection_name;
}

void ParupaintConnection::setAutoJoinFlag(bool f)
{
	autojoin_flag = f;
}
bool ParupaintConnection::autoJoinFlag() const
{
	return autojoin_flag;
}

QWsSocket * ParupaintConnection::socket()
{
	return this->connection_socket;
}

bool ParupaintConnection::isLocal()
{
	return (this->socket()->host() == "localhost");
}
