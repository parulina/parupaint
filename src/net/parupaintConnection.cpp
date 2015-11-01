#include "parupaintConnection.h"
#include <QJsonDocument>
#include <QDebug>

ParupaintConnection::ParupaintConnection(QWebSocket * s) : socket(s), id(0)
{
}

qint64 ParupaintConnection::send(const QString id, const QJsonObject &obj)
{
	return this->send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

qint64 ParupaintConnection::send(const QString id, const QString msg)
{
	if(!this->socket) return 0;
	return socket->sendTextMessage(id + " " + msg);
}
void ParupaintConnection::setId(sid id)
{
	this->id = id;
}
sid ParupaintConnection::getId() const
{
	return this->id;
}

QWebSocket * ParupaintConnection::getSocket()
{
	return this->socket;
}
