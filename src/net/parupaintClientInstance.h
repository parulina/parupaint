#ifndef PARUPAINTCLIENTINSTANCE_H
#define PARUPAINTCLIENTINSTANCE_H

#include <QHash>
#include <QJsonObject>

#include "parupaintClient.h"
#include "../core/parupaintBrush.h"

class ParupaintVisualCursor;
class ParupaintCanvasScene;
class ParupaintBrush;

class ParupaintClientInstance : public ParupaintClient
{
Q_OBJECT
	bool playmode;
	int me;
	QString nickname;
	ParupaintCanvasScene * pool;
	// has to work with int because the server is the only connection
	QHash<quint32, ParupaintVisualCursor*> brushes;
	ParupaintBrush shadow_brush;


	public:
	ParupaintClientInstance(ParupaintCanvasScene*, QObject * = nullptr);
	virtual void send(const QString , const QJsonObject & = QJsonObject());
	void message(const QString &, const QByteArray &);

	void ReloadImage();
	void ReloadCanvas();
	void PlayRecord(QString, bool as_script);
	void SaveCanvas(const QString);
	void LoadCanvas(const QString);
	void FillCanvas(int, int, QString);
	void NewCanvas(int, int, bool=false);
	void LoadCanvasLocal(const QString);
	void SendLayerFrame(int, int, int=0, int=0, bool=false);
	void SendBrushUpdate(ParupaintBrush * brush);
	void PasteLayerFrameImage(int l, int f, int x, int y, QImage);

	void SetNickname(QString);
	void SendChat(const QString & = QString());

	signals:
	void ChatMessageReceived(QString, QString);
	void OnConnect();
	void OnDisconnect(QString reason = "");
	void PlaymodeUpdate(ParupaintBrush *);
};

#endif
