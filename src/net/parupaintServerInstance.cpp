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
	canvas = new ParupaintPanvas(this, QSize(540, 540), 1, 1);
	this->ServerFill(0, 0, "#FFF");

	this->StartRecordSystems();
	// Start record systems before doing ANYTHING

	/*
	auto * con = new ParupaintConnection(nullptr);
	con->setId(connectid++);
	this->ServerJoin(con, "test");
	QTimer *timer = new QTimer(this);
	connect(timer, &QTimer::timeout, [=](){
		auto * b = this->brushes[con];
		QPointF add(4, 4);
		if(b->GetPosition().x() > 500 && b->GetPosition().y() > 500){
			add = QPointF(-500, -500);
		}
		b->SetPosition(b->GetPosition() + add);
		QJsonObject obj;
		obj["id"] = con->getId();
		obj["x"] = rand() % 255;
		obj["y"] = rand() % 255;
		obj["t"] = 1;
		this->Broadcast("draw", obj);
	});
	timer->start(800);
	*/
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QString ParupaintServerInstance::MarshalCanvas()
{
	QJsonObject obj;
	obj["width"] = canvas->dimensions().width();
	obj["height"] = canvas->dimensions().height();

	qDebug() << "MarshalCanvas" << obj << canvas->layerCount();
	QJsonArray layers;
	for(auto l = 0; l < canvas->layerCount(); l++){
		auto * layer = canvas->layerAt(l);
		if(!layer) continue;

		qDebug() << " - l" << l << "frameCount" << layer->frameCount();
		QJsonArray frames;
		for(auto f = 0; f < layer->frameCount(); f++){
			auto * frame = layer->frameAt(f);


			QJsonObject fobj;
			fobj["extended"] = !(layer->isFrameReal(f));
			fobj["opacity"] = frame->opacity();
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

	obj["name"] = brushes[connection]->name();
	obj["x"] = brushes[connection]->x();
	obj["y"] = brushes[connection]->y();
	obj["w"] = brushes[connection]->size();
	obj["t"] = brushes[connection]->tool();
	obj["l"] = brushes[connection]->layer();
	obj["f"] = brushes[connection]->frame();

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

