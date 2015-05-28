#ifndef PARUPAINTCLIENTINSTANCE_H
#define PARUPAINTCLIENTINSTANCE_H

#include <QHash>
#include <QJsonObject>

#include "parupaintClient.h"

class ParupaintCanvasBrush;
class ParupaintCanvasPool;
class ParupaintBrush;

class ParupaintClientInstance : public ParupaintClient
{
Q_OBJECT
	int me;
	QString nickname;
	ParupaintCanvasPool * pool;
	// has to work with int because the server is the only connection
	QHash<quint32, ParupaintCanvasBrush*> brushes;

	public:
	ParupaintClientInstance(ParupaintCanvasPool*, QObject * = nullptr);
	virtual void send(const QString , const QJsonObject & = QJsonObject());
	void ReloadImage();
	void SaveCanvas(const QString);
	void LoadCanvas(const QString);
	void SendLayerFrame(int, int, int=0, int=0, bool=false);
	void SendBrushUpdate(ParupaintBrush * brush);

	void SetNickname(QString);
	void SendChat(QString);

	private slots:
	void Message(const QString, const QByteArray);

	signals:
	void ChatMessageReceived(QString, QString);
};

#endif
