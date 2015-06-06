#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPen> // brush draw

// LoadCanvasLocal
#include <QFile> 
#include <QFileInfo>

#include "parupaintClientInstance.h"
#include "../parupaintCanvasPool.h"
#include "../parupaintCanvasObject.h"

#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintStrokeStep.h"

#include "../parupaintCanvasBrush.h"
#include "../core/parupaintBrush.h"

#include "qcompressor.h"

#include <QDebug>

ParupaintClientInstance::ParupaintClientInstance(ParupaintCanvasPool * p, QObject * parent) : ParupaintClient(parent)
{
	me = -1;
	pool = p;
	connect(this, &ParupaintClient::onMessage, this, &ParupaintClientInstance::Message);
}

void ParupaintClientInstance::Message(const QString id, const QByteArray bytes)
{
	QJsonObject object = QJsonDocument::fromJson(bytes).object();

	if(id == "connect"){
		
		QJsonObject obj;
		obj["name"] = nickname;
		this->send("join", obj);

	} else if(id == "join"){

	} else if(id == "peer") { // TODO join this with join pls?
		auto c = object["id"].toInt();
		auto d = object["disconnect"].toBool();
		auto n = object["name"].toString();

		if(!d) {
			auto idd = (c < 0) ? -c : c;
			brushes[idd] = new ParupaintCanvasBrush;
			brushes[idd]->SetPressure(-1); // use width only
			if(c < 0) me = idd;


			if(c > 0) pool->AddCursor(n, brushes[c]);
		} else {
			if(brushes.find(c) != brushes.end()){
				pool->RemoveCursor(brushes[c]);
				pool->ClearBrushStrokes(brushes[c]);
				delete brushes[c];
				brushes.remove(c);
			}
		}

	} else if(id == "lf") {
		auto c = object["id"].toInt();
		auto * brush = brushes.value(c);
		if(brush) {
			auto l = object["l"].toInt();
			auto f = object["f"].toInt();
			brush->SetLayer(l);
			brush->SetFrame(f);
			if(c == me){
				pool->GetCanvas()->SetLayerFrame(l, f);
				pool->TriggerViewUpdate();
			}
		}

	} else if(id == "draw"){
		auto c = object["id"].toInt();
		auto * brush = brushes.value(c);
		if(brush) {

			QColor color = HexToColor(object["c"].toString());
			auto drawing(object["d"].toBool());
			auto width(object["s"].toDouble());
			auto x(object["x"].toDouble());
			auto y(object["y"].toDouble());

			brush->SetColor(color);
			brush->SetWidth(width);

			if(drawing){
				if(!brush->GetCurrentStroke()) pool->NewBrushStroke(brush);

				if(brush->GetCurrentStroke()) {
					auto * step = new ParupaintStrokeStep(*brush);
					brush->GetCurrentStroke()->AddStroke(step);
				}

			} else {
				if(brush->IsDrawing()){
					if(brush->GetCurrentStroke()){
						pool->EndBrushStroke(brush);
						pool->SquashBrushStrokes(brush);
					}
					pool->GetCanvas()->TriggerCacheRedraw();
				}
			}

			brush->SetPosition(QPointF(x, y));
			brush->SetDrawing(drawing);
			pool->TriggerViewUpdate();
		}
	} else if (id == "canvas") {
		auto w = object["width"].toInt();
		auto h = object["height"].toInt();
		auto layers = object["layers"].toArray();

		auto canvas = pool->GetCanvas();
		canvas->Clear();
		auto ll = 0;
		foreach(auto l, layers){
			canvas->AddLayers(ll, 1, 0);
			auto *canvas_layer = canvas->GetLayer(ll);
			if(!canvas_layer) continue;

			auto ff = 0;
			foreach(auto f, l.toArray()){
				auto frame = f.toObject();
				auto extended = frame["extended"].toBool();
				auto opacity = frame["opacity"].toDouble();

				// extended can't be the first. what
				if(extended && ff == 0) extended = false;

				if(extended){
					canvas_layer->ExtendFrame(ff-1);
				} else {
					canvas_layer->AddFrames(ff, 1);
					canvas_layer->GetFrame(ff)->SetOpacity(opacity);
				}


				ff++;
			}

			ll++;
		}
		// resizes and updates CanvasView
		// it crashed here because it was using old LF vals
		canvas->Resize(QSize(w, h));
		// updates view and flayer list
		pool->TriggerViewUpdate();
		// fixes and sets currently selected in flayer list
		pool->GetCanvas()->FixLayerFrame();
		// sends a reload signal
		this->ReloadImage();


	} else if (id == "img") {

		auto l = object["l"].toInt();
		auto f = object["f"].toInt();
		auto w = object["w"].toInt();
		auto h = object["h"].toInt();

		QByteArray compressed = QByteArray::fromBase64(object["data"].toString().toLatin1());
		QByteArray uncompressed;
		QCompressor::gzipDecompress(compressed, uncompressed);

		auto * layer = pool->GetCanvas()->GetLayer(l);
		if(layer){
			auto * frame = layer->GetFrame(f);
			if(frame){
				QImage img(w, h, QImage::Format_ARGB32);
				memcpy(img.bits(), uncompressed, img.byteCount());
				frame->Replace(img);
			}
		}
		pool->GetCanvas()->TriggerCacheRedraw();
		pool->UpdateView();
	} else if(id == "chat") {
		auto name = object["name"].toString(),
		     msg = object["message"].toString();
		emit ChatMessageReceived(name, msg);
	} else {
		//qDebug() << id << object;
	}
}

void ParupaintClientInstance::ReloadImage()
{
	this->ParupaintClient::send("img");
}
void ParupaintClientInstance::SendLayerFrame(int layer, int frame, int ll, int ff, bool ext)
{
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
	QJsonObject obj;
	obj["x"] = brush->GetPosition().x();
	obj["y"] = brush->GetPosition().y();
	obj["s"] = brush->GetWidth() * brush->GetPressure();
	obj["c"] = brush->GetColorString();
	obj["d"] = brush->IsDrawing();
	this->send("draw", obj);
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

void ParupaintClientInstance::SetNickname(QString str)
{
	nickname = str;
}

void ParupaintClientInstance::SendChat(QString str)
{
	QJsonObject obj;
	obj["message"] = str;
	obj["name"] = nickname;
	this->send("chat", obj);
}

void ParupaintClientInstance::send(const QString id, const QJsonObject & obj)
{
	this->ParupaintClient::send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
