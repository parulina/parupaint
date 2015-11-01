#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include <QtWebSockets/QWebSocket>

typedef int sid;

class ParupaintConnection
{
	private:
	QWebSocket * socket;
	sid id;

	public:
	ParupaintConnection(QWebSocket * s);
	qint64 send(const QString id, const QJsonObject &obj);
	qint64 send(const QString id, const QString msg);

	void setId(sid);
	sid getId() const;

	QWebSocket * getSocket();
};

#endif
