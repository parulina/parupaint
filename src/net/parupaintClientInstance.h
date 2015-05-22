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
	ParupaintClientInstance(ParupaintCanvasPool*,QUrl, QObject * = nullptr);
	void ReloadImage();
	void SendLayerFrame(ParupaintBrush * brush);
	void SendBrushUpdate(ParupaintBrush * brush);

	private slots:
	void Message(QString, const QByteArray);
};

#endif
