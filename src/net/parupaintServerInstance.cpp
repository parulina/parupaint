#include "parupaintServerInstance.h"

#include <QSettings>
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
	QSettings server_cfg("server.ini", QSettings::IniFormat, this);
	this->server_password = server_cfg.value("password").toString();

	canvas = new ParupaintPanvas(this, QSize(540, 540), 1, 1);

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

void ParupaintServerInstance::setPassword(const QString & password)
{
	this->server_password = password;
}

const QString & ParupaintServerInstance::password()
{
	return this->server_password;
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QJsonObject ParupaintServerInstance::MarshalConnection(ParupaintConnection* connection)
{
	QJsonObject obj;
	obj["id"] = connection->id();

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
	this->sendAll("chat", obj);
}

QJsonObject ParupaintServerInstance::canvasObj()
{
	QJsonObject obj;

	obj["w"] = canvas->dimensions().width();
	obj["h"] = canvas->dimensions().height();

	QJsonArray layers;
	for(int l = 0; l < canvas->layerCount(); l++){
		ParupaintLayer * layer = canvas->layerAt(l);
		if(!layer) continue;

		QJsonArray frames;
		for(int f = 0; f < layer->frameCount(); f++){
			ParupaintFrame * frame = layer->frameAt(f);

			QJsonObject fobj;
			fobj["extended"] = !(layer->isFrameReal(f));
			fobj["opacity"] = frame->opacity();
			frames.insert(f, fobj);
		}
		layers.insert(l, frames);
	}
	obj["layers"] = layers;

	return obj;
}

void ParupaintServerInstance::sendAll(const QString & id, const QJsonObject & obj, ParupaintConnection * con)
{
	foreach(ParupaintConnection * c, this->ppConnections()){
		if(c == con) continue;
		c->send(id, obj);
	}
}

