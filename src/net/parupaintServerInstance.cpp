#include "parupaintServerInstance.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "parupaintConnection.h"
#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintBrush.h"

ParupaintServerInstance::~ParupaintServerInstance()
{
	this->StopRecordSystems();
}

ParupaintServerInstance::ParupaintServerInstance(quint16 port, QObject * parent) : ParupaintServer(port, parent),
	canvas(nullptr), connectid(1),
	record_player(nullptr), record_manager(nullptr)
{
	canvas = new ParupaintPanvas(540, 540, 1);
	this->ServerFill(0, 0, "#FFF");

	this->StartRecordSystems();
	// Start record systems before doing ANYTHING

	connect(this, &ParupaintServer::onMessage, this, &ParupaintServerInstance::Message);
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QString ParupaintServerInstance::MarshalCanvas()
{
	QJsonObject obj;
	obj["width"] = canvas->GetWidth();
	obj["height"] = canvas->GetHeight();

	QJsonArray layers;
	for(auto l = 0; l < canvas->GetNumLayers(); l++){
		auto * layer = canvas->GetLayer(l);
		if(!layer) continue;

		QJsonArray frames;
		for(auto f = 0; f < layer->GetNumFrames(); f++){
			auto * frame = layer->GetFrame(f);

			QJsonObject fobj;
			fobj["extended"] = !(layer->IsFrameReal(f));
			fobj["opacity"] = frame->GetOpacity();

			frames.insert(f, fobj);
		}
		layers.insert(l, frames);
	}
	obj["layers"] = layers;
		

	return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QJsonObject ParupaintServerInstance::MarshalConnection(ParupaintConnection* connection)
{
	QJsonObject obj;
	obj["id"] = connection->getId();

	obj["name"] = brushes[connection]->GetName();
	obj["x"] = brushes[connection]->GetPosition().x();
	obj["y"] = brushes[connection]->GetPosition().y();
	obj["w"] = brushes[connection]->GetWidth();
	
	return obj;
}

int ParupaintServerInstance::GetNumConnections()
{
	return brushes.size();
}
void ParupaintServerInstance::BroadcastChat(QString str)
{
	QJsonObject obj;
	obj["message"] = str;
	this->Broadcast("chat", obj);
}

void ParupaintServerInstance::Broadcast(QString id, QJsonObject ba, ParupaintConnection * c)
{
	this->Broadcast(id, QJsonDocument(ba).toJson(QJsonDocument::Compact), c);
}
void ParupaintServerInstance::Broadcast(QString id, QString ba, ParupaintConnection * c)
{
	return this->Broadcast(id, ba.toUtf8(), c);
}
void ParupaintServerInstance::Broadcast(QString id, const QByteArray ba, ParupaintConnection * c)
{
	foreach(auto i, brushes.keys()){
		if(i != c){
			if(i->getSocket()) i->send(id, ba);
		}
	}
}

