#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include <QWebSocket>

struct ParupaintConnection
{
	QWebSocket * socket;
	int id;

	ParupaintConnection(QWebSocket * s) : socket(s), id(0) {}
	qint64 send(QString, QString);
	qint64 sendBinary(QString, const QByteArray&);
};

#endif
