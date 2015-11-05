#ifndef PARUPAINTCLIENT_H
#define PARUPAINTCLIENT_H

#include "ws/QWsSocket.h"

#include <QUrl>
#include <QObject>

class ParupaintClient : public QObject
{
Q_OBJECT
	private:
	QWsSocket socket;
	QString host;
	quint16 port;
	bool Connected;
	bool SwitchHost;

	public:
	ParupaintClient(QObject * = nullptr);
	void Connect(QString = "localhost");
	void Connect(QString, quint16);
	void Disconnect(QString reason = "");

	qint64 send(const QString id, const QString data);
	qint64 send(const QString data);

	QUrl url();

	private slots:
	void onConnect();
	void onDisconnect();
	void textReceived(QString);
	void onError(QAbstractSocket::SocketError);
	void onSocketStateChanged(QAbstractSocket::SocketState socketState);

signals:
	void onMessage(QString, const QByteArray=QByteArray());
};


#endif
