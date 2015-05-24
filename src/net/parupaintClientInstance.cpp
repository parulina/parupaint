#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPen> // brush draw

#include "parupaintClientInstance.h"
#include "../parupaintCanvasPool.h"
#include "../parupaintCanvasObject.h"

#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"

#include "../parupaintCanvasBrush.h"
#include "../core/parupaintBrush.h"

#include "qcompressor.h"

#include <QDebug>

ParupaintClientInstance::ParupaintClientInstance(ParupaintCanvasPool * p, QObject * parent) : ParupaintClient(QUrl("localhost"), parent)
{
	me = -1;
	pool = p;
	connect(this, &ParupaintClient::onMessage, this, &ParupaintClientInstance::Message);
}

void ParupaintClientInstance::Message(const QString id, const QByteArray bytes)
{
	QJsonObject object = QJsonDocument::fromJson(bytes).object();

	if(id == "connect"){

		this->send("join");

	} else if(id == "join"){

	} else if(id == "peer") { // TODO join this with join pls?
		auto c = object["id"].toInt();
		auto d = object["disconnect"].toBool();
		auto n = object["name"].toString();

		if(!d) {
			auto idd = (c < 0) ? -c : c;
			brushes[idd] = new ParupaintCanvasBrush;
			if(c < 0) me = idd;


			if(c > 0) pool->AddCursor(n, brushes[c]);
		} else {
			if(brushes.find(c) != brushes.end()){
				pool->RemoveCursor(brushes[c]);
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
			}
		}

	} else if(id == "draw"){
		auto c = object["id"].toInt();
		auto * brush = brushes.value(c);
		if(brush) {

			QColor color(object["c"].toString().left(7));
			auto drawing(object["d"].toBool());
			auto width(object["s"].toDouble());
			auto x(object["x"].toDouble());
			auto y(object["y"].toDouble());

			auto old_x = brush->GetPosition().x();
			auto old_y = brush->GetPosition().y();
			auto l = brush->GetLayer();
			auto f = brush->GetFrame();

			brush->SetColor(color);
			brush->SetWidth(width);

			if(drawing){
				auto * layer = pool->GetCanvas()->GetLayer(l);
				if(layer) {
					auto * frame = layer->GetFrame(f);
					if(frame){
						frame->DrawStep(old_x, old_y, x, y, width, color);
					}
				}
			}

			brush->SetPosition(QPointF(x, y));
			brush->SetDrawing(drawing);
			pool->UpdateView();
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
					qDebug() << "Extending frame " << ff-1;
					canvas_layer->ExtendFrame(ff-1);
				} else {
					canvas_layer->AddFrames(ff, 1);
					canvas_layer->GetFrame(ff)->SetOpacity(opacity);
				}


				ff++;
			}

			ll++;
		}
		canvas->Resize(QSize(w, h));
		pool->UpdateView();
		pool->GetCanvas()->SetLayerFrame(0, 0);
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
				qDebug() << "Replacing image for" << l << f;
				QImage img(w, h, QImage::Format_ARGB32);
				memcpy(img.bits(), uncompressed, img.byteCount());
				frame->Replace(img);
			}
		}
		pool->UpdateView();

	} else {
		qDebug() << id << object;
	}
}

void ParupaintClientInstance::ReloadImage()
{
	this->ParupaintClient::send("img");
}
void ParupaintClientInstance::SendLayerFrame(ParupaintBrush * brush)
{
	QJsonObject obj;
	obj["l"] = brush->GetLayer();
	obj["f"] = brush->GetFrame();
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


void ParupaintClientInstance::send(const QString id, const QJsonObject & obj)
{
	this->ParupaintClient::send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}
