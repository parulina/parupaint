#ifndef PARUPAINTCLIENT_H
#define PARUPAINTCLIENT_H

#include <QtWebSockets/QtWebSockets>
#include <QUrl>
#include <QObject>

typedef QWebSocket ParupaintWebSocket;

class ParupaintClient : public QObject
{
Q_OBJECT
	private:
	ParupaintWebSocket socket;
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
