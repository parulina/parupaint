#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include "ws/QWsSocket.h"

typedef int sid;

class ParupaintConnection
{
	private:
	QWsSocket * socket;
	sid id;

	public:
	ParupaintConnection(QWsSocket * s);
	qint64 send(const QString id, const QJsonObject &obj);
	qint64 send(const QString id, const QString msg);

	void setId(sid);
	sid getId() const;

	QWsSocket * getSocket();
};

#endif
