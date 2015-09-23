#ifndef PARUPAINTCLIENTINSTANCE_H
#define PARUPAINTCLIENTINSTANCE_H

#include <QHash>
#include <QJsonObject>

#include "parupaintClient.h"

class ParupaintCanvasBrush;
class ParupaintCanvasPool;
class ParupaintBrush;

enum DrawMode {
	DRAW_MODE_DIRECT,
	DRAW_MODE_SIMPLE,
	DRAW_MODE_UNDO,
};
class ParupaintClientInstance : public ParupaintClient
{
Q_OBJECT
	bool playmode;
	int me;
	QString nickname;
	ParupaintCanvasPool * pool;
	// has to work with int because the server is the only connection
	QHash<quint32, ParupaintCanvasBrush*> brushes;
	DrawMode DrawMethod;
	ParupaintBrush * shadow_brush;

	public:
	ParupaintClientInstance(ParupaintCanvasPool*, QObject * = nullptr);
	~ParupaintClientInstance();
	virtual void send(const QString , const QJsonObject & = QJsonObject());
	void ReloadImage();
	void ReloadCanvas();
	void PlayRecord(QString, bool as_script);
	void SaveCanvas(const QString);
	void LoadCanvas(const QString);
	void NewCanvas(int, int, bool=false);
	void LoadCanvasLocal(const QString);
	void SendLayerFrame(int, int, int=0, int=0, bool=false);
	void SendBrushUpdate(ParupaintBrush * brush);

	void SetNickname(QString);
	void SendChat(QString);

	DrawMode GetDrawMode() const;

	private slots:
	void Message(const QString, const QByteArray);

	signals:
	void ChatMessageReceived(QString, QString);
	void OnDisconnect();
	void PlaymodeUpdate(ParupaintBrush *);
};

#endif
