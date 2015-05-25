#ifndef PARUPAINTCLIENTINSTANCE_H
#define PARUPAINTCLIENTINSTANCE_H

#include <QHash>

#include "parupaintClient.h"

class ParupaintCanvasBrush;
class ParupaintCanvasPool;
class ParupaintBrush;

class ParupaintClientInstance : public ParupaintClient
{
	int me;
	ParupaintCanvasPool * pool;
	// has to work with int because the server is the only connection
	QHash<quint32, ParupaintCanvasBrush*> brushes;

	public:
	ParupaintClientInstance(ParupaintCanvasPool*, QObject * = nullptr);
	virtual void send(const QString , const QJsonObject & = QJsonObject());
	void ReloadImage();
	void SaveCanvas(const QString);
	void LoadCanvas(const QString);
	void SendLayerFrame(int, int);
	void SendBrushUpdate(ParupaintBrush * brush);

	private slots:
	void Message(const QString, const QByteArray);
};

#endif
