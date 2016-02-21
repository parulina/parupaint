#include "parupaintClientInstance.h"

#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

#include "../core/parupaintBrush.h"
#include "../core/parupaintSnippets.h"
#include "../core/parupaintFrameBrushOps.h"
#include "../core/parupaintCommonOperations.h"
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

		// TODO clean this up??
		// this is to make sure that, even if you reconnect
		// you will send fresh data to the new server
		shadow_brush.setPressure(0);
		shadow_brush.setColor(QColor(-1, -1, -1, -1));
		shadow_brush.setSize(-1);
		shadow_brush.setTool(-1);
		shadow_brush.setDrawing(false);

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

			if(object["n"].isString())	brush->setCursorName(object["n"].toString("Unnamed"));

			QVariantMap map;
			if(object["x"].isDouble()) map["x"] = object["x"].toDouble();
			if(object["y"].isDouble()) map["y"] = object["y"].toDouble();
			if(object["p"].isDouble()) map["p"] = object["p"].toDouble();
			if(object["s"].isDouble()) map["s"] = object["s"].toDouble();
			if(object["l"].isDouble()) map["l"] = object["l"].toInt();
			if(object["f"].isDouble()) map["f"] = object["f"].toInt();
			if(object["t"].isDouble()) map["t"] = object["t"].toInt();
			if(object["d"].isBool())   map["d"] = object["d"].toBool();
			if(object["c"].isString()) map["c"] = ParupaintSnippets::toColor(object["c"].toString());

			QLineF draw_line;
			ParupaintCommonOperations::BrushOp(brush, draw_line, map);
			brush->QGraphicsItem::setPos(brush->position());

			if(brush->drawing()){
				QRect r = ParupaintFrameBrushOps::stroke(pool->canvas(), brush, draw_line);
				pool->canvas()->redraw(r);
			}
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

		ParupaintCommonOperations::LayerFramePasteOp(pool->canvas(), l, f, x, y, image);
		pool->canvas()->redraw();

	} else if (id == "fill") {
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;
		if(!object["c"].isString()) return;
		int l = object["l"].toInt(),
		    f = object["f"].toInt();
		QColor c = ParupaintSnippets::toColor(object["c"].toString());

		ParupaintCommonOperations::LayerFrameFillOp(pool->canvas(), l, f, c);
		pool->canvas()->redraw();

	} else if (id == "new") {
		if(!object["w"].isDouble()) return;
		if(!object["h"].isDouble()) return;
		if(!object["r"].isBool()) return;

		bool r = object["r"].toBool();
		int w = object["w"].toInt(),
		    h = object["h"].toInt();

		ParupaintCommonOperations::CanvasResizeOp(pool->canvas(), w, h, r);
		pool->canvas()->redraw();

	} else if (id == "canvas") {
		pool->canvas()->loadJson(object);

		// reload all images
		this->doReloadImage();

	} else if (id == "image") {

		int l = object["l"].toInt(),
		    f = object["f"].toInt(),
		    w = object["w"].toInt(),
		    h = object["h"].toInt();

		QByteArray compressed = QByteArray::fromBase64(object["data"].toString().toLatin1());
		QByteArray uncompressed;
		QCompressor::gzipDecompress(compressed, uncompressed);

		ParupaintLayer * layer = pool->canvas()->layerAt(l);
		if(layer){
			ParupaintFrame * frame = layer->frameAt(f);
			if(frame){
				QImage img(w, h, QImage::Format_ARGB32);
				memcpy(img.bits(), uncompressed, img.byteCount());
				frame->replaceImage(img);
			}
		}
		// todo do this after receiving all images only?
		pool->canvas()->redraw();

	} else if(id == "lfc") {
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;
		if(!object["lc"].isDouble()) return;
		if(!object["fc"].isDouble()) return;

		int l = object["l"].toInt(),
		    f = object["f"].toInt(),
		    lc = object["lc"].toInt(),
		    fc = object["fc"].toInt();
		// TODO change ext -> extended
		bool ext = object["ext"].toBool();

		ParupaintCommonOperations::LayerFrameChangeOp(pool->canvas(), l, f, lc, fc, ext);

		pool->canvas()->setCurrentLayerFrame(pool->canvas()->currentLayer(), pool->canvas()->currentFrame());
		pool->canvas()->redraw();

	} else if(id == "lfa") {
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;
		if(!object["attr"].isObject()) return;

		QJsonObject attr = object["attr"].toObject();
		if(!attr.length()) return;

		int l = object["l"].toInt(),
		    f = object["f"].toInt();

		foreach(const QString & key, attr.keys()){
			const QVariant & val = attr[key].toVariant();
			qDebug() << key << val;
			ParupaintCommonOperations::LayerFrameAttributeOp(pool->canvas(), l, f, key, val);
		}
		pool->canvas()->redraw();

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
