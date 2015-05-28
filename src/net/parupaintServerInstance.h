#ifndef PARUPAINTSERVERINSTANCE_H
#define PARUPAINTSERVERINSTANCE_H

// This is like a virtual server or whatever...

#include <QHash>

#include "parupaintServer.h"

class ParupaintBrush;
class ParupaintPanvas;

class ParupaintServerInstance : public ParupaintServer
{
	ParupaintPanvas * canvas;
	QHash<ParupaintConnection*, ParupaintBrush*> brushes;
	int connectid;

	public:
	ParupaintServerInstance(quint16 , QObject * = nullptr);
	void Broadcast(QString, QJsonObject, ParupaintConnection * = nullptr);
	void Broadcast(QString, QString, ParupaintConnection * = nullptr);
	void Broadcast(QString, const QByteArray, ParupaintConnection * = nullptr);
	
	QString MarshalCanvas();
	ParupaintPanvas * GetCanvas();
	int GetNumConnections();

	private slots:
	void Message(ParupaintConnection *, const QString, const QByteArray);
};

#endif
