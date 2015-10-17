#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include "../bundled/qtwebsocket/QWsSocket.h"

typedef int sid;

class ParupaintConnection
{
	private:
	QtWebsocket::QWsSocket * socket;
	sid id;

	public:
	ParupaintConnection(QtWebsocket::QWsSocket * s);
	qint64 send(const QString id, const QJsonObject &obj);
	qint64 send(const QString id, const QString msg);

	void setId(sid);
	sid getId() const;

	QtWebsocket::QWsSocket * getSocket();
};

#endif
