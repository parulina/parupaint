#include "parupaintClientInstance.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPen> // brush draw
#include <QBuffer>

// LoadCanvasLocal
#include <QFile> 
#include <QFileInfo>


#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintSnippets.h"

#include "../bundled/qcompressor.h"

ParupaintClientInstance::ParupaintClientInstance(ParupaintCanvasScene * p, QObject * parent) : ParupaintClient(parent)
{
	playmode = false;
	me = -1;
	pool = p;
}

void ParupaintClientInstance::send(const QString id, const QJsonObject & obj)
{
	this->ParupaintClient::send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

void ParupaintClientInstance::ReloadImage()
{
	this->ParupaintClient::send("img");
}
void ParupaintClientInstance::ReloadCanvas()
{
	this->ParupaintClient::send("canvas");
}
void ParupaintClientInstance::SendLayerFrame(int layer, int frame, int ll, int ff, bool ext)
{
	if(playmode) return;
	QJsonObject obj;
	obj["l"] = layer;
	obj["f"] = frame;
	obj["ll"] = ll;
	obj["ff"] = ff;
	obj["ext"] = ext;
	this->send("lf", obj);
}


void ParupaintClientInstance::SendBrushUpdate(ParupaintBrush * brush)
{
	if(playmode) return;
	QJsonObject obj;
	if(brush->position() != shadow_brush.position()){
		obj["x"] = brush->x();
		obj["y"] = brush->y();
	}
	if(brush->layer() != shadow_brush.layer())
		obj["l"] = brush->layer();

	if(brush->frame() != shadow_brush.frame())
		obj["f"] = brush->frame();

	if(brush->size() != shadow_brush.size())
		obj["w"] = brush->size();

	if(brush->pressure() != shadow_brush.pressure())
		obj["p"] = brush->pressure();

	if(brush->color() != shadow_brush.color())
		obj["c"] = brush->colorString();

	if(brush->drawing() != shadow_brush.drawing())
		obj["d"] = brush->drawing();

	if(brush->tool() != shadow_brush.tool())
		obj["t"] = brush->tool();

	if(obj.length()){
		this->send("draw", obj);
	}
	brush->copyTo(shadow_brush);
}

void ParupaintClientInstance::PasteLayerFrameImage(int l, int f, int x, int y, QImage img)
{
	QJsonObject obj;
	obj["layer"] = l;
	obj["frame"] = f;
	obj["x"] = x;
	obj["y"] = y;
	obj["image"] = ParupaintSnippets::ImageToBase64Gzip(img);
	this->send("paste", obj);
}

void ParupaintClientInstance::LoadCanvasLocal(const QString filename)
{
	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) return;
	QByteArray compressed;
	QCompressor::gzipCompress(file.readAll(), compressed);
	
	QJsonObject obj;
	obj["file"] = QString(compressed.toBase64());
	obj["filename"] = QFileInfo(filename).fileName();
	this->send("load", obj);
}
void ParupaintClientInstance::LoadCanvas(const QString filename)
{
	QJsonObject obj;
	obj["filename"] = filename;
	this->send("load", obj);
}
void ParupaintClientInstance::SaveCanvas(const QString filename)
{
	QJsonObject obj;
	obj["filename"] = filename;
	this->send("save", obj);
}

void ParupaintClientInstance::NewCanvas(int w, int h, bool resize)
{
	QJsonObject obj;
	obj["width"] = w;
	obj["height"] = h;
	obj["resize"] = resize;
	this->send("new", obj);
}

void ParupaintClientInstance::FillCanvas(int l, int f, QString col)
{
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = col;
	this->send("fill", obj);
}

void ParupaintClientInstance::PlayRecord(QString filename, bool as_script)
{
	if(filename.isEmpty()) return;

	QJsonObject obj;
	obj["filename"] = filename;
	obj["as_script"] = as_script;
	this->send("play", obj);
}

void ParupaintClientInstance::SetNickname(QString str)
{
	nickname = str;
}

void ParupaintClientInstance::SendChat(const QString & str)
{
	QJsonObject obj;
	if(!str.isEmpty()){
		obj["message"] = str;
		obj["name"] = nickname;
	}
	this->send("chat", obj);
}

