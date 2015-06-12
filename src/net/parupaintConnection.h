#ifndef PARUPAINTCONNECTION_H
#define PARUPAINTCONNECTION_H

#include "../bundled/qtwebsocket/QWsSocket.h"

struct ParupaintConnection
{
	QtWebsocket::QWsSocket * socket;
	int id;

	ParupaintConnection(QtWebsocket::QWsSocket * s) : socket(s), id(0) {}
	qint64 send(QString, QString);
	qint64 sendBinary(QString, const QByteArray&);
};

#endif
