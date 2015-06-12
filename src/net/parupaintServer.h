#ifndef PARUPAINTSERVER_H
#define PARUPAINTSERVER_H

#include <QList>
#include <QObject>
#include <QAbstractSocket>

class ParupaintConnection;
namespace QtWebsocket{
	class QWsSocket;
	class QWsServer;
}
// this includes websocket

class ParupaintServer : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintConnection*> connections;
	QtWebsocket::QWsServer *server;

	ParupaintConnection * GetConnection(QtWebsocket::QWsSocket*);

	private slots:
	void onConnection();
	void onDisconnection();
	void textReceived(QString);
	void onError(QAbstractSocket::SocketError);

	public:
	ParupaintServer(quint16 port, QObject * = nullptr);
	~ParupaintServer();

	signals:
	void onMessage(ParupaintConnection*, QString, const QByteArray = "");
};


#endif
