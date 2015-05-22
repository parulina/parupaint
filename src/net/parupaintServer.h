#ifndef PARUPAINTSERVER_H
#define PARUPAINTSERVER_H

#include <QAbstractSocket>
#include <QList>
#include <QObject>

class QWebSocketServer;
class ParupaintConnection;
class QWebSocket;
// this includes websocket

class ParupaintServer : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintConnection*> connections;
	QWebSocketServer *server;

	ParupaintConnection * GetConnection(QWebSocket*);

	private slots:
	void onConnection();
	void onDisconnection();
	void textReceived(QString);
	void onError(QAbstractSocket::SocketError);

	public:
	ParupaintServer(quint16 port, QObject * = nullptr);
	~ParupaintServer();

	signals:
	void onMessage(ParupaintConnection*, QString, const QByteArray);
};


#endif
