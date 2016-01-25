#include "parupaintClientInstance.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPen> // brush draw
#include <QBuffer>
#include <QSettings>

// LoadCanvasLocal
#include <QFile> 
#include <QFileInfo>


#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintSnippets.h"

#include "../bundled/qcompressor.h"
#include "../parupaintVersion.h"

ParupaintClientInstance::ParupaintClientInstance(ParupaintCanvasScene * p, QObject * parent) :
	ParupaintClient(parent),
	read_only(false), client_joined(false), me(-1),
	remote_password(false),
	pool(p)
{
	QSettings cfg;
	this->setName(cfg.value("client/username").toString());
}

void ParupaintClientInstance::send(const QString id, const QJsonObject & obj)
{
	this->ParupaintClient::send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}


void ParupaintClientInstance::setName(const QString & str)
{
	this->client_name = str;
}
void ParupaintClientInstance::setReadOnly(bool r)
{
	this->read_only = r;
}

const QString ParupaintClientInstance::name()
{
	return this->client_name.isEmpty() ? "Noname" : this->client_name;
}
bool ParupaintClientInstance::readOnly()
{
	return this->read_only;
}
bool ParupaintClientInstance::isJoined()
{
	return client_joined;
}
bool ParupaintClientInstance::remoteHasPassword()
{
	return remote_password;
}



void ParupaintClientInstance::doJoin(const QString & password)
{
	QJsonObject obj;
	obj["version"] = PARUPAINT_VERSION;
	if(!password.isEmpty()){
		obj["password"] = password;
	}
	this->send("join", obj);
}

void ParupaintClientInstance::doLeave()
{
	this->send("leave");
}

void ParupaintClientInstance::doName()
{
	// server works with a separate name packet.
	QJsonObject obj;
	obj["name"] = this->name();
	this->send("name", obj);
}

void ParupaintClientInstance::doReloadCanvas()
{
	this->send("canvas");
}

void ParupaintClientInstance::doReloadImage(int l, int f)
{
	QJsonObject obj;
	if(l != -1) obj["l"] = l;
	if(f != -1) obj["f"] = f;
	this->send("image", obj);
}

void ParupaintClientInstance::doChat(const QString & str)
{
	QJsonObject obj;
	if(!str.isEmpty()){
		obj["message"] = str;
		obj["name"] = this->name();
	}
	this->send("chat", obj);
}

void ParupaintClientInstance::doBrushUpdate(ParupaintBrush * brush)
{
	if(!this->isJoined()) return;
	if(this->readOnly()) return;

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
		obj["s"] = brush->size();

	if(brush->pressure() != shadow_brush.pressure())
		obj["p"] = brush->pressure();

	if(brush->color() != shadow_brush.color())
		obj["c"] = brush->colorString();

	if(brush->drawing() != shadow_brush.drawing())
		obj["d"] = brush->drawing();

	if(brush->tool() != shadow_brush.tool())
		obj["t"] = brush->tool();

	if(obj.length()){
		this->send("brush", obj);
	}
	brush->copyTo(shadow_brush);
}

void ParupaintClientInstance::doLayerFrameAttribute(int l, int f, const QString & attr, const QJsonValue & val)
{
	if(this->readOnly()) return;

	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["attr"] = QJsonObject{
		{attr, val}
	};
	this->send("lfa", obj);
}
void ParupaintClientInstance::doLayerFrameChange(int l, int f, int lc, int fc, bool ext)
{
	if(this->readOnly()) return;

	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["lc"] = lc;
	obj["fc"] = fc;
	obj["ext"] = ext;
	this->send("lfc", obj);
}

void ParupaintClientInstance::doPasteImage(int l, int f, int x, int y, const QImage & img)
{
	if(this->readOnly()) return;

	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["x"] = x;
	obj["y"] = y;
	obj["image"] = ParupaintSnippets::ImageToBase64Gzip(img);
	this->send("paste", obj);
}

void ParupaintClientInstance::doFill(int l, int f, const QString & col)
{
	if(this->readOnly()) return;
	if(col.isEmpty()) return;

	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = col;
	this->send("fill", obj);
}
void ParupaintClientInstance::doNew(int w, int h, bool resize)
{
	if(this->readOnly()) return;

	// 2 ^ 13
	if(w >= 8192 || h >= 8192) return;
	if(w <=    0 || h <=    0) return;

	QJsonObject obj;
	obj["w"] = w;
	obj["h"] = h;
	obj["r"] = resize;
	this->send("new", obj);
}

void ParupaintClientInstance::doInfo(const QString & attr, const QVariant & val)
{
	if(this->readOnly()) return;

	QJsonObject obj = {
		{attr, QJsonValue::fromVariant(val)}
	};
	this->send("info", obj);
}

void ParupaintClientInstance::doLoadLocal(const QString & filename)
{
	if(this->readOnly()) return;

	QFile file(filename);
	if(!file.open(QIODevice::ReadOnly)) return;

	QByteArray compressed;
	QCompressor::gzipCompress(file.readAll(), compressed);
	if(compressed.isEmpty()) return;

	QJsonObject obj;
	obj["file"] = QString(compressed.toBase64());
	obj["filename"] = QFileInfo(filename).fileName();
	this->send("load", obj);
}
void ParupaintClientInstance::doLoad(const QString & filename)
{
	if(this->readOnly()) return;
	if(filename.isEmpty()) return;

	QJsonObject obj;
	obj["filename"] = filename;
	this->send("load", obj);
}
void ParupaintClientInstance::doSave(const QString & filename)
{
	if(this->readOnly()) return;
	if(filename.isEmpty()) return;

	QJsonObject obj;
	obj["filename"] = filename;
	this->send("save", obj);
}

/*
void ParupaintClientInstance::PlayRecord(QString filename, bool as_script)
{
	if(filename.isEmpty()) return;

	QJsonObject obj;
	obj["filename"] = filename;
	obj["as_script"] = as_script;
	this->send("play", obj);
}
*/
