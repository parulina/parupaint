#include "parupaintClientInstance.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

#include "../core/parupaintBrush.h"
#include "../core/parupaintSnippets.h"
#include "../core/parupaintFrameBrushOps.h"
#include "../bundled/qcompressor.h"
#include "../parupaintVersion.h"

#include "../parupaintVisualCursor.h"
#include "../parupaintVisualCanvas.h"
#include "../parupaintCanvasScene.h"

void ParupaintClientInstance::message(const QString & id, const QByteArray & bytes)
{
	const QJsonObject object = QJsonDocument::fromJson(bytes).object();

	if(id == "connect"){

		QJsonObject obj;
		obj["name"] = nickname;
		obj["version"] = PARUPAINT_VERSION;
		this->send("join", obj);
		emit OnConnect();

		this->ReloadCanvas();

	} else if(id == "disconnect"){
		emit OnDisconnect(bytes);

	} else if(id == "peer") {
		auto c = object["id"].toInt();
		auto d = object["disconnect"].toBool();

		auto n(object["name"].toString());
		auto x(object["x"].toDouble());
		auto y(object["y"].toDouble());
		auto w(object["w"].toDouble());

		if(c > 0){
			if(!d) {
				brushes[c] = new ParupaintVisualCursor();
				brushes[c]->setName(n);
				brushes[c]->setSize(w);
				brushes[c]->setPosition(QPointF(x, y));

				brushes[c]->setCursorName(n);

				pool->addCursor(brushes[c]); // addCursor is needed here
			} else {
				if(brushes.find(c) != brushes.end()){
					ParupaintVisualCursor * cursor = brushes.take(c);
					pool->removeCursor(cursor);
					delete cursor;
				}
			}

		} else {
			me = -c;
		}
	} else if(id == "draw"){
		auto c = object["id"].toInt();
		if(c == this->me) return;
		if(brushes.find(c) == brushes.end()) return;

		ParupaintVisualCursor * brush = brushes.value(c);
		if(brush) {
			const qreal 	old_x = brush->ParupaintBrush::x(),
			      		old_y = brush->ParupaintBrush::y();
			double 		x = old_x, y = old_y;

			if(object["c"].isString())	brush->setColor(ParupaintSnippets::toColor(object["c"].toString("#000")));
			if(object["d"].isBool())	brush->setDrawing(object["d"].toBool(false));
			if(object["w"].isDouble())	brush->setSize(object["w"].toDouble(1));
			if(object["p"].isDouble())	brush->setPressure(object["p"].toDouble(0.0));
			if(object["t"].isDouble())	brush->setTool(object["t"].toInt(0));
			if(object["l"].isDouble())	brush->setLayer(object["l"].toInt(0));
			if(object["f"].isDouble())	brush->setFrame(object["f"].toInt(0));

			if(object["x"].isDouble())	x = object["x"].toDouble();
			if(object["y"].isDouble())	y = object["y"].toDouble();

			if(brush->drawing()){
				QRect r = ParupaintFrameBrushOps::stroke(pool->canvas(), brush, QPointF(x, y), QPointF(old_x, old_y));
				pool->canvas()->redraw(r);
			}
			brush->setPosition(QPointF(x, y));
			brush->update();
		}
	} else if (id == "paste") {
		if(!object["paste"].isString()) return;
		QImage image = ParupaintSnippets::Base64GzipToImage(object["paste"].toString());
		if(image.isNull()) return;

		if(object["layer"].isDouble() && object["frame"].isDouble()){
			int l = object["layer"].toInt(),
			    f = object["frame"].toInt(),
			    x = object["x"].toInt(),
			    y = object["y"].toInt();
			ParupaintLayer * layer = pool->canvas()->layerAt(l);
			if(layer){
				ParupaintFrame * frame = layer->frameAt(f);
				if(frame){
					QRect rect = frame->drawImage(QPointF(x, y), image);
					pool->canvas()->redraw(rect);
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
		ParupaintLayer * layer = pool->canvas()->layerAt(l);
		if(layer){
			ParupaintFrame * frame = layer->frameAt(f);
			if(frame){
				frame->clear(c);
			}
		}
		pool->canvas()->redraw();

	} else if (id == "canvas") {
		int w = object["width"].toInt();
		int h = object["height"].toInt();
		QJsonArray layers = object["layers"].toArray();

		ParupaintVisualCanvas* canvas = pool->canvas();
		canvas->clearCanvas();
		canvas->resize(QSize(w, h));

		auto ll = 0;
		foreach(auto l, layers){
			canvas->insertLayer(0, 0);
			auto *canvas_layer = canvas->layerAt(ll);
			if(!canvas_layer) continue;

			auto ff = 0;
			foreach(auto f, l.toArray()){
				auto frame = f.toObject();
				auto extended = frame["extended"].toBool();
				auto opacity = frame["opacity"].toDouble();

				// extended can't be the first. what
				if(extended && ff == 0) extended = false;

				if(extended){
					canvas_layer->extendFrame(ff-1);
				} else {
					canvas_layer->insertFrame(canvas->dimensions(), ff);
					canvas_layer->frameAt(ff)->setOpacity(opacity);
				}


				ff++;
			}

			ll++;
		}
		canvas->setCurrentLayerFrame(canvas->currentLayer(), canvas->currentFrame());
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

		auto * layer = pool->canvas()->layerAt(l);
		if(layer){
			auto * frame = layer->frameAt(f);
			if(frame){
				QImage img(w, h, QImage::Format_ARGB32);
				memcpy(img.bits(), uncompressed, img.byteCount());
				frame->replaceImage(img);
			}
		}
		// todo do this after receiving all images only?
		pool->canvas()->redraw();

	} else if(id == "chat") {
		const QString name = object["name"].toString(),
		              message = object["message"].toString();
		if(object["message"].isString()){
			emit ChatMessageReceived(message, name);
		}

		int id = object["id"].toInt();
		if(id == this->me) return;

		if(brushes.find(id) == brushes.end()) return;
		ParupaintVisualCursor * brush = brushes.value(id);
		if(brush){
			if(message.isEmpty()){
				brush->setStatus(ParupaintVisualCursorStatus::CursorStatusTyping, 2000);
			} else {
				brush->setStatus(ParupaintVisualCursorStatus::CursorStatusNone, 0);
			}
			brush->update();
		}

	} else if(id == "play") {
		if(!object["count"].isDouble()) return;
		int count = object["count"].toInt();
		playmode = (bool)(count);

	} else {
		//qDebug() << id << object;
	}
}
