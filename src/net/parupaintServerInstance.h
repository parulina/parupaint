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

	QHash<ParupaintConnection*, ParupaintBrush> record_backup;
	QTimer record_timer;

	ParupaintRecordPlayer * record_player;
	ParupaintRecordManager * record_manager;

	ParupaintPanvas * canvas;
	QHash<ParupaintConnection*, ParupaintBrush*> brushes;
	int connectid;

	// returns true if reached end
	bool ProcessRecord(QByteArray & ba, int steps);
	void StartRecordTimer();
	void RecordTimerStep();

	void SaveRecordBrushes();
	void RestoreRecordBrushes();

	public:
	~ParupaintServerInstance();
	ParupaintServerInstance(quint16 , QObject * = nullptr);

	void RecordLineDecoder(const QString & line, bool recovery=false);

	void ServerJoin(ParupaintConnection *, QString, bool=true);
	void ServerLeave(ParupaintConnection *, bool=true);
	void ServerChat(ParupaintConnection *, QString, bool=true);

	// special - uses no id
	void ServerLfc(int l, int f, int lc, int fc, bool e, bool=true);
	void ServerFill(int l, int f, QString, bool=true);
	void ServerPaste(int l, int f, int x, int y, QString base64_img, bool propagate=true);
	void ServerResize(int, int, bool, bool=true);

	void Broadcast(QString, QJsonObject, ParupaintConnection * = nullptr);
	void Broadcast(QString, QString, ParupaintConnection * = nullptr);
	void Broadcast(QString, const QByteArray, ParupaintConnection * = nullptr);

	void BroadcastChat(QString);
	
	QString MarshalCanvas();
	QJsonObject MarshalConnection(ParupaintConnection*);
	ParupaintPanvas * GetCanvas();
	int GetNumConnections();

	private slots:
	void Message(ParupaintConnection *, const QString, const QByteArray);

	signals:
	void OnMessage(const QString & id, const QJsonObject &obj);
};

#endif
