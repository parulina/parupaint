#ifndef PARUPAINTSERVERINSTANCE_H
#define PARUPAINTSERVERINSTANCE_H

// This is like a virtual server or whatever...

#include <QHash>
#include <QTimer>
#include <QDir>

#include "../core/parupaintRecordManager.h"
#include "parupaintServer.h"

class ParupaintBrush;
class ParupaintPanvas;

class ParupaintServerInstance : public ParupaintServer
{
Q_OBJECT
	QByteArray log_recovery;

	QHash<ParupaintConnection*, ParupaintBrush*> record_backup;
	QTimer record_timer;

	ParupaintPanvas * canvas;
	QHash<ParupaintConnection*, ParupaintBrush*> brushes;
	int connectid;
	QString server_password;
	QDir ppweb_serve;
	QDir server_dir;

	ParupaintRecordManager record_manager;

	protected:
	void objMessage(const QString & id, const QJsonObject & obj);

	private slots:
	void browserVisit(QTcpSocket * socket, const QString & path);

	public:
	~ParupaintServerInstance();
	ParupaintServerInstance(quint16 port, QObject * = nullptr);

	void startRecord();
	void backupState();
	void restoreState();
	void playLogFile(const QString & logfile, int limit = -1);
	void doLine(const QString & line);
	void doLines(const QStringList & lines);

	void joinConnection(ParupaintConnection * con);
	void leaveConnection(ParupaintConnection * con);
	ParupaintConnection * getConnection(int id);
	void setBrushesDrawing(bool stopdraw=false);

	void sendInfo();
	void doMessage(const QString & id, QJsonObject obj);

	void setParupaintWebServeDir(QDir dir);
	const QDir & parupaintWebServeDir() const;

	void setServerDir(QDir dir);
	const QDir & serverDir() const;

	void setPassword(const QString & password);
	const QString & password();

	virtual void message(ParupaintConnection *, const QString &, const QByteArray & = QByteArray());

	QJsonObject connectionObj(ParupaintConnection * con) const;
	QJsonObject canvasObj() const;

	void sendAll(const QString &, const QJsonObject &, ParupaintConnection * = nullptr);
	void sendChat(const QString &, ParupaintConnection * = nullptr);

	int numSpectators() const;
	int numPainters() const;
	int numConnections() const;
	
	ParupaintPanvas * GetCanvas();

	signals:
	void OnMessage(const QString & id, const QJsonObject &obj);
	void onJoin(ParupaintConnection *);
	void onLeave(ParupaintConnection *);
	void onConnect(ParupaintConnection *);
	void onDisconnect(ParupaintConnection *);
};

#endif
