
#include "parupaintConnection.h"

qint64 ParupaintConnection::sendBinary(QString i, const QByteArray & m)
{
	return socket->sendBinaryMessage(QString(i + " " + m).toUtf8());
}

qint64 ParupaintConnection::send(QString i, QString m)
{
	return socket->sendTextMessage(i + " " + m);
}
