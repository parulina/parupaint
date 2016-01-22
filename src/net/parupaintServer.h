#ifndef PARUPAINTSERVER_H
#define PARUPAINTSERVER_H

#include <QList>
#include <QObject>
#include <QAbstractSocket>

// QWsSocket
class ParupaintConnection;
class QWsServer;
class QWsSocket;

class ParupaintServer : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintConnection*> connections;
	QWsServer* server;
	bool protective;


	private slots:
	void onConnection();
	void onDisconnection();
	void textReceived(QString);

	public:
	ParupaintServer(quint16 port, QObject * = nullptr);
	~ParupaintServer();

	virtual void message(ParupaintConnection *, const QString &, const QByteArray & = QByteArray()) {};

	void setProtective(bool);
	bool isProtective();

	// clean this filth up
	ParupaintConnection * ppConnection(QWsSocket*);
	QList<ParupaintConnection*> ppConnections();
	int ppNumConnections() const;
};


#endif
