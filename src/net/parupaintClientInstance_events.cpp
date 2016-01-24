#include "parupaintClientInstance.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

#include "../core/parupaintBrush.h"
#include "../core/parupaintSnippets.h"
#include "../core/parupaintFrameBrushOps.h"
#include "../bundled/qcompressor.h"

#include "../parupaintVisualCursor.h"
#include "../parupaintVisualCanvas.h"
#include "../parupaintCanvasScene.h"

void ParupaintClientInstance::message(const QString & id, const QByteArray & bytes)
{
	const QJsonObject object = QJsonDocument::fromJson(bytes).object();

	if(id == "connect"){
		emit onConnect();
		if(this->url().host() != "localhost"){
			emit onSpectateChange(!(client_joined = false));
		}

		this->doName();
		if(this->url().host() == "localhost"){
			this->doJoin();
		}

	} else if(id == "disconnect"){
		emit onDisconnect(bytes);

	// we got confirmation
	} else if(id == "join"){
		emit onSpectateChange(!(client_joined = true));

	} else if(id == "leave"){
		emit onSpectateChange(!(client_joined = false));

	} else if(id == "info") {
		if(object["password"].isBool()){
			remote_password = object["password"].toBool(false);
		}

	} else if(id == "brush"){
		int c = object["id"].toInt();
		if(c == this->me) return;

		// default: nullptr
		ParupaintVisualCursor * brush = brushes[c];

		// create or destroy brush as they join/leave the server
		if(object["exists"].isBool()){
			bool exists = object["exists"].toBool();
			if(exists){
				brush = new ParupaintVisualCursor;
				pool->addCursor(brush);
				brushes[c] = brush;
			} else if(brush) {
				brush = brushes.take(c);
				pool->removeCursor(brush);
				delete brush;
				brush = nullptr;
			}
		}
		if(brush) {
			const qreal 	old_x = brush->ParupaintBrush::x(),
			      		old_y = brush->ParupaintBrush::y();
			double 		x = old_x, y = old_y;

			if(object["n"].isString())	brush->setCursorName(object["n"].toString("Unnamed"));
			if(object["c"].isString())	brush->setColor(ParupaintSnippets::toColor(object["c"].toString("#000")));
			if(object["d"].isBool())	brush->setDrawing(object["d"].toBool(false));
			if(object["s"].isDouble())	brush->setSize(object["s"].toDouble(1));
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
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;

		QImage image = ParupaintSnippets::Base64GzipToImage(object["paste"].toString());
		if(image.isNull()) return;

		int l = object["l"].toInt(),
		    f = object["f"].toInt(),
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
		if(!object["w"].isDouble()) return;
		if(!object["h"].isDouble()) return;

		int w = object["w"].toInt();
		int h = object["h"].toInt();
		QJsonArray layers = object["layers"].toArray();

		ParupaintVisualCanvas* canvas = pool->canvas();
		canvas->clearCanvas();
		canvas->setBackgroundColor(ParupaintSnippets::toColor(object["bgc"].toString("#00000000")));
		canvas->resize(QSize(w, h));

		for(int i = 0; i < layers.size(); i++){
			canvas->insertLayer(0, 0);
		}

		int ll = 0;
		foreach(QJsonValue l, layers){
			ParupaintLayer *canvas_layer = canvas->layerAt(ll);
			if(!canvas_layer) continue;

			int ff = 0;
			foreach(QJsonValue f, l.toArray()){
			QJsonObject frame = f.toObject();
				bool extended = frame["extended"].toBool();
				qreal opacity = frame["opacity"].toDouble();

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

		// reload all images
		this->doReloadImage();

	} else if (id == "image") {

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

	} else if(id == "lfc") {
		this->send("canvas");

	} else if(id == "lfa") {
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;
		if(!object["attr"].isObject()) return;

		int l = object["l"].toInt(), f = object["f"].toInt();
		QJsonObject attr = object["attr"].toObject();

		ParupaintLayer * layer = pool->canvas()->layerAt(l);
		if(!layer) return;
		ParupaintFrame * frame = layer->frameAt(f);
		if(!frame) return;

		if(!attr.length()) return;
		foreach(const QString & key, attr.keys()){
			const QVariant & val = attr[key].toVariant();

			if(key == "frame-opacity"){
				frame->setOpacity(val.toDouble());
				pool->canvas()->redraw();
			}
		}

	} else if(id == "chat") {
		const QString name = object["name"].toString(),
		              message = object["message"].toString();
		if(object["message"].isString()){
			emit onChatMessage(message, name);
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

	} else {
		//qDebug() << id << object;
	}
}
