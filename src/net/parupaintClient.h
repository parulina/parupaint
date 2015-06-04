#ifndef PARUPAINTCLIENT_H
#define PARUPAINTCLIENT_H

#include "QtWebsocket/QWsSocket.h"
#include <QUrl>
#include <QObject>

class ParupaintClient : public QObject
{
Q_OBJECT
	private:
	QtWebsocket::QWsSocket socket;
	QString host;
	quint16 port;
	bool Connected;
	bool SwitchHost;

	public:
	ParupaintClient(QObject * = nullptr);
	void Connect(QString = "", quint16 = 0);
	void Disconnect();

	void send(QString id, QString data = "");

	private slots:
	void onConnect();
	void onDisconnect();
	void textReceived(QString);
	void onError(QAbstractSocket::SocketError);
	void onSocketStateChanged(QAbstractSocket::SocketState socketState);

signals:
	void onMessage(QString, const QByteArray);
};


#endif
