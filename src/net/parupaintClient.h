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
	QUrl url;
	bool Connected;

	public:
	ParupaintClient(QString, QObject * = nullptr);
	void Connect(QString = "");
	void Disconnect();

	void send(QString id, QString data = "");

	private slots:
	void onConnect();
	void onDisconnect();
	void textReceived(QString);
	void onError(QAbstractSocket::SocketError);

signals:
	void onMessage(QString, const QByteArray);
};


#endif
