#ifndef PARUPAINTSERVERINSTANCE_H
#define PARUPAINTSERVERINSTANCE_H

// This is like a virtual server or whatever...

#include <QHash>
#include <QTimer>

#include "parupaintServer.h"

class ParupaintBrush;
class ParupaintPanvas;
class ParupaintRecordManager;
class ParupaintRecordPlayer;

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

	protected:
	// Record related things (parupaintServerInstance.rec.cpp)
	ParupaintRecordPlayer * record_player;
	ParupaintRecordManager * record_manager;

	void StopRecordSystems();
	void StartRecordSystems();

	void StartRecordTimer();
	void RecordTimerStep();
	void SaveRecordBrushes();
	void RestoreRecordBrushes();

	void RecordLineDecoder(const QString & line, bool recovery=false);
	// play directly to canvas, no updates
	void PlayRecordLog(const QString & file);
	// end record related things

	public:
	~ParupaintServerInstance();
	ParupaintServerInstance(quint16 , QObject * = nullptr);

	void setPassword(const QString & password);
	const QString & password();

	virtual void message(ParupaintConnection *, const QString &, const QByteArray & = QByteArray());

	void ServerJoin(ParupaintConnection *, bool=true);
	void ServerLeave(ParupaintConnection *, bool=true);
	void ServerName(ParupaintConnection *, QString, bool=true);
	void ServerChat(ParupaintConnection *, QString, bool=true);

	// special - uses no id
	void ServerLfa(int l, int f, const QString & attr, const QVariant & val, bool=true);
	void ServerLfc(int l, int f, int lc, int fc, bool e, bool=true);
	void ServerFill(int l, int f, QString, bool=true);
	void ServerPaste(int l, int f, int x, int y, QImage img, bool propagate=true);
	void ServerPaste(int l, int f, int x, int y, QString base64_img, bool propagate=true);
	void ServerResize(int, int, bool, bool=true);

	QJsonObject canvasObj();
	void sendAll(const QString &, const QJsonObject &, ParupaintConnection * = nullptr);

	void BroadcastChat(QString);
	
	QJsonObject MarshalConnection(ParupaintConnection*);
	ParupaintPanvas * GetCanvas();
	int GetNumConnections();

	signals:
	void OnMessage(const QString & id, const QJsonObject &obj);
	void onJoin(ParupaintConnection *);
	void onLeave(ParupaintConnection *);
};

#endif
