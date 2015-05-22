#ifndef PARUPAINTCLIENT_H
#define PARUPAINTCLIENT_H

#include <QAbstractSocket>
#include <QWebSocket>
#include <QUrl>
#include <QObject>


class ParupaintClient : public QObject
{
Q_OBJECT
	private:
	QWebSocket socket;
	QUrl url;
	public:
	ParupaintClient(QUrl, QObject * = nullptr);
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
