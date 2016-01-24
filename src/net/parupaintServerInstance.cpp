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
#include "../core/parupaintSnippets.h"

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
}

void ParupaintServerInstance::joinConnection(ParupaintConnection * con)
{
	con->send("join");
	emit onJoin(con);

	this->ServerJoin(con, true);
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

QJsonObject ParupaintServerInstance::connectionObj(ParupaintConnection * con) const
{
	QJsonObject obj = {
		{"id", con->id()},
		{"n", con->name()}
	};

	ParupaintBrush * brush = this->brushes.value(con);
	if(brush){
		obj["x"] = brush->x();
		obj["y"] = brush->y();
		obj["s"] = brush->size();
		obj["t"] = brush->tool();
		obj["l"] = brush->layer();
		obj["f"] = brush->frame();
	}

	return obj;
}

QJsonObject ParupaintServerInstance::canvasObj() const
{
	QJsonObject obj;

	obj["bgc"] = ParupaintSnippets::toHex(canvas->backgroundColor());
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
void ParupaintServerInstance::sendChat(const QString & str, ParupaintConnection * con)
{
	QJsonObject obj;
	obj["message"] = str;
	this->sendAll("chat", obj, con);
}

int ParupaintServerInstance::numSpectators() const
{
	return (this->numConnections() - this->numPainters());
}

int ParupaintServerInstance::numPainters() const
{
	return brushes.size();
}

int ParupaintServerInstance::numConnections() const
{
	return this->ppNumConnections();
}
