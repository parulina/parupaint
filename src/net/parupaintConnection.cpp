
#include "parupaintConnection.h"

qint64 ParupaintConnection::sendBinary(QString i, const QByteArray & m)
{
	return 0; //socket->sendBinaryMessage(QString(i + " " + m).toUtf8());
}

qint64 ParupaintConnection::send(QString i, QString m)
{
	if(socket) return socket->write(i + " " + m);
	return 0;
}
