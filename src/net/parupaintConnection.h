#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include "ws/QWsSocket.h"

class ParupaintConnection
{
	private:
	QWsSocket * connection_socket;
	int connection_id;
	QString connection_name;
	bool autojoin_flag;

	public:
	ParupaintConnection(QWsSocket * s);
	qint64 send(const QString id, const QJsonObject &obj);
	qint64 send(const QString id, const QString msg = QString());

	void setId(int id);
	int id() const;

	void setName(const QString &);
	QString name();

	void setAutoJoinFlag(bool f);
	bool autoJoinFlag() const;

	QWsSocket * socket();
	bool isLocal();
};

#endif
