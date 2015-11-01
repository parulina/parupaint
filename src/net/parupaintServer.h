#ifndef PARUPAINTSERVER_H
#define PARUPAINTSERVER_H

#include <QList>
#include <QObject>
#include <QAbstractSocket>

// QWebSocket
class ParupaintConnection;
class QWebSocketServer;
class QWebSocket;

typedef QWebSocketServer ParupaintWebSocketServer;
// this includes websocket

class ParupaintServer : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintConnection*> connections;
	ParupaintWebSocketServer* server;

	ParupaintConnection * GetConnection(QWebSocket*);

	private slots:
	void onConnection();
	void onDisconnection();
	void textReceived(QString);

	public:
	ParupaintServer(quint16 port, QObject * = nullptr);
	~ParupaintServer();

	signals:
	void onMessage(ParupaintConnection*, QString, const QByteArray = "");
};


#endif
