#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPen> // brush draw
#include <QBuffer>

// LoadCanvasLocal
#include <QFile> 
#include <QFileInfo>

#include "parupaintClientInstance.h"
#include "../parupaintCanvasPool.h"
#include "../parupaintCanvasObject.h"

#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintStrokeStep.h"
#include "../core/parupaintSnippets.h"

#include "../parupaintCanvasBrush.h"
#include "../core/parupaintBrush.h"

#include "../core/parupaintFrameBrushOps.h"
#include "../parupaintVersion.h"

#include "qcompressor.h"

#include <QDebug>

ParupaintClientInstance::ParupaintClientInstance(ParupaintCanvasPool * p, QObject * parent) : ParupaintClient(parent)
{
	playmode = false;
	me = -1;
	DrawMethod = DRAW_MODE_DIRECT;
	pool = p;
	shadow_brush = new ParupaintBrush();
	connect(this, &ParupaintClient::onMessage, this, &ParupaintClientInstance::Message);
}
ParupaintClientInstance::~ParupaintClientInstance()
{
	delete shadow_brush;
}

void ParupaintClientInstance::Message(const QString id, const QByteArray bytes)
{
	const QJsonObject object = QJsonDocument::fromJson(bytes).object();

	if(id == "connect"){
		
		qDebug("Connect success");
		QJsonObject obj;
		obj["name"] = nickname;
		obj["version"] = PARUPAINT_VERSION;
		this->send("join", obj);

	} else if(id == "disconnect"){
		emit OnDisconnect(bytes);

	} else if(id == "peer") {
		auto c = object["id"].toInt();
		auto d = object["disconnect"].toBool();

		auto n(object["name"].toString());
		auto x(object["x"].toDouble());
		auto y(object["y"].toDouble());
		auto w(object["w"].toDouble());

		if(!d) {
			auto idd = (c < 0) ? -c : c;
			brushes[idd] = new ParupaintCanvasBrush;
			brushes[idd]->SetName(n);
			brushes[idd]->SetPosition(QPointF(x, y));
			brushes[idd]->SetWidth(w);

			brushes[idd]->ShowName(-1);
			if(c < 0) {
				me = idd;
				this->ReloadCanvas();
			}


			if(c > 0) pool->AddCursor(n, brushes[c]);
		} else {
			if(brushes.find(c) != brushes.end()){
				pool->RemoveCursor(brushes[c]);
				pool->ClearBrushStrokes(brushes[c]);
				delete brushes[c];
				brushes.remove(c);
			}
		}
	} else if(id == "draw"){
		auto c = object["id"].toInt();
		if(c == this->me) return;
		if(brushes.find(c) == brushes.end()) return;

		auto * brush = brushes.value(c);
		if(brush) {
			const double 	old_x = brush->GetPosition().x(),
			      		old_y = brush->GetPosition().y();
			double 		x = old_x, y = old_y;

			if(object["c"].isString())	brush->SetColor(ParupaintSnippets::toColor(object["c"].toString()));
			if(object["d"].isBool())	brush->SetDrawing(object["d"].toBool(false));
			if(object["w"].isDouble())	brush->SetWidth(object["w"].toDouble(1));
			if(object["p"].isDouble())	brush->SetPressure(object["p"].toDouble(0.0));
			if(object["t"].isDouble())	brush->SetToolType(object["t"].toInt(0));
			if(object["l"].isDouble())	brush->SetLayer(object["l"].toInt(0));
			if(object["f"].isDouble())	brush->SetFrame(object["f"].toInt(0));

			if(object["x"].isDouble())	x = object["x"].toDouble();
			if(object["y"].isDouble())	y = object["y"].toDouble();
			if(brush->IsDrawing()){
				if(DrawMethod == DRAW_MODE_DIRECT){
					QRect r = ParupaintFrameBrushOps::stroke(pool->GetCanvas(), 
							old_x, old_y, x, y, brush);
					pool->GetCanvas()->RedrawCache(r);
					if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill){
						brush->SetDrawing(false);
					}

				} else {
					if(!brush->GetCurrentStroke()) pool->NewBrushStroke(brush);
					if(brush->GetCurrentStroke()) {
						auto * step = new ParupaintStrokeStep(*brush);
						brush->GetCurrentStroke()->AddStroke(step);
					}
				}

			} else {
				if(brush->IsDrawing()){
					// doesn't matter, this will never
					// evaluate since it'll never be created.
					// still looks kinda ugly though.

					if(brush->GetCurrentStroke()){
						pool->EndBrushStroke(brush);
						pool->SquashBrushStrokes(brush);
					}
				}
			}
			brush->SetPosition(QPointF(x, y));
			if(playmode && c == me) {
				emit PlaymodeUpdate(brush);
			}
		}
	} else if (id == "paste") {
		if(!object["paste"].isString()) return;
		QImage image = ParupaintSnippets::toImage(object["paste"].toString());
		if(image.isNull()) return;

		if(object["layer"].isDouble() && object["frame"].isDouble()){
			int l = object["layer"].toInt(),
			    f = object["frame"].toInt(),
			    x = object["x"].toInt(),
			    y = object["y"].toInt();
			ParupaintLayer * layer = pool->GetCanvas()->GetLayer(l);
			if(layer){
				ParupaintFrame * frame = layer->GetFrame(f);
				if(frame){
					frame->DrawImage(x, y, image);
					pool->GetCanvas()->RedrawCache();
					pool->TriggerViewUpdate();
				}
			}
		}
	} else if (id == "fill") {
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;
		if(!object["c"].isString()) return;
		int l = object["l"].toInt(),
		    f = object["f"].toInt();
		QColor c = ParupaintSnippets::toColor(object["c"].toString());
		pool->GetCanvas()->Fill(l, f, c);

		pool->GetCanvas()->RedrawCache();
		pool->TriggerViewUpdate();

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
		// todo do this after receiving all images only?
		pool->GetCanvas()->RedrawCache();
		pool->UpdateView();

	} else if(id == "chat") {
		auto name = object["name"].toString(),
		     msg = object["message"].toString();
		emit ChatMessageReceived(name, msg);

	} else if(id == "play") {
		if(!object["count"].isDouble()) return;
		int count = object["count"].toInt();
		playmode = (bool)(count);

	} else {
		//qDebug() << id << object;
	}
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
	if(brush->GetPosition() != shadow_brush->GetPosition()){
		obj["x"] = brush->GetPosition().x();
		obj["y"] = brush->GetPosition().y();
	}
	if(brush->GetLayer() != shadow_brush->GetLayer()){
		obj["l"] = brush->GetLayer();
	}
	if(brush->GetFrame() != shadow_brush->GetFrame()){
		obj["f"] = brush->GetFrame();
	}
	if(brush->GetWidth() != shadow_brush->GetWidth()){
		obj["w"] = brush->GetWidth();
	}
	if(brush->GetPressure() != shadow_brush->GetPressure()){
		obj["p"] = brush->GetPressure();
	}
	if(brush->GetColor() != shadow_brush->GetColor()){
		obj["c"] = brush->GetColorString();
	}
	if(brush->IsDrawing() != shadow_brush->IsDrawing()){
		obj["d"] = brush->IsDrawing();
	}
	if(brush->GetToolType() != shadow_brush->GetToolType()){
		obj["t"] = brush->GetToolType();
	}
	if(obj.length()) this->send("draw", obj);
	*shadow_brush = *brush;
}

void ParupaintClientInstance::PasteLayerFrameImage(int l, int f, int x, int y, QImage img)
{
	QByteArray pngData;
	QBuffer buf(&pngData);
	buf.open(QIODevice::WriteOnly);
	img.save(&buf, "png");
	buf.close();

	QByteArray compressed;
	QCompressor::gzipCompress(pngData, compressed);

	QJsonObject obj;
	obj["layer"] = l;
	obj["frame"] = f;
	obj["x"] = x;
	obj["y"] = y;
	obj["image"] = "data:image/png;base64," + QString(compressed.toBase64());
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

DrawMode ParupaintClientInstance::GetDrawMode() const
{
	return DrawMethod;
}
