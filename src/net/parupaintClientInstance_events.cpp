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
		pool->clearCursors();

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
		if(object["painters"].isDouble()){
			emit onPlayerCountChange(object["painters"].toInt());
		}
		if(object["spectators"].isDouble()){
			emit onSpectatorCountChange(object["spectators"].toInt());
		}
		if(object["password"].isBool()){
			remote_password = object["password"].toBool(false);
		}
		foreach(const QString & key, object.keys()){
			QVariant val = object[key].toVariant();
			if(key == "project-bgc" && val.type() == QVariant::String) val = QColor(val.toString());
			ParupaintCommonOperations::CanvasAttributeOp(pool->canvas(), key, val);
		}

	} else if(id == "brush"){
		int c = object["id"].toInt();
		if(c == this->me) return;

		// default: nullptr
		ParupaintVisualCursor * brush = brushes.value(c);

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
			if(object["st"].isDouble())	brush->setStatus(object["st"].toInt(), 2000);

			QVariantMap map;
			if(object["x"].isDouble()) map["x"] = object["x"].toDouble();
			if(object["y"].isDouble()) map["y"] = object["y"].toDouble();
			if(object["p"].isDouble()) map["p"] = object["p"].toDouble();
			if(object["s"].isDouble()) map["s"] = object["s"].toDouble();
			if(object["l"].isDouble()) map["l"] = object["l"].toInt();
			if(object["f"].isDouble()) map["f"] = object["f"].toInt();
			if(object["t"].isDouble()) map["t"] = object["t"].toInt();
			if(object["a"].isDouble()) map["a"] = object["a"].toInt();
			if(object["d"].isBool())   map["d"] = object["d"].toBool();
			if(object["c"].isString()) map["c"] = QColor(object["c"].toString());

			QLineF draw_line;
			double old_size = brush->pressureSize();
			ParupaintCommonOperations::BrushOp(brush, draw_line, map);
			brush->QGraphicsItem::setPos(brush->position());

			if(brush->drawing()){
				QRect r = ParupaintFrameBrushOps::stroke(pool->canvas(), brush, draw_line, old_size);
				pool->canvas()->redraw(r);
			}
			brush->update();
		}
	} else if (id == "paste") {
		if(!object["image"].isString()) return;
		if(!object["l"].isDouble()) return;
		if(!object["f"].isDouble()) return;

		QImage image = ParupaintSnippets::Base64GzipToImage(object["image"].toString());
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
		QColor c(object["c"].toString());

		ParupaintCommonOperations::LayerFrameFillOp(pool->canvas(), l, f, c);
		pool->canvas()->redraw();

	} else if (id == "canvas") {
		if(object["resize"].isBool() && object["resize"].toBool(false)){
			int w = object["canvasWidth"].toInt(), h = object["canvasHeight"].toInt();
			ParupaintCommonOperations::CanvasResizeOp(pool->canvas(), w, h, true);
			pool->canvas()->redraw();
		} else {
			pool->canvas()->loadJson(object);
			this->doReloadImage();
			pool->canvas()->adjustCurrentLayerFrame();
		}

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
		bool ext = object["ext"].toBool();

		if(ParupaintCommonOperations::LayerFrameChangeOp(pool->canvas(), l, f, lc, fc, ext)){
			foreach(ParupaintBrush * brush, this->brushes){
				ParupaintCommonOperations::AdjustBrush(brush, pool->canvas());
			}
		}

		ParupaintBrush * brush = pool->mainCursor();

		if(l-1 <= brush->layer()) {
			brush->setLayer(brush->layer() + lc);
		}
		if(f-1 <= brush->frame()) {
			brush->setFrame(brush->frame() + fc);
		}

		pool->canvas()->setCurrentLayerFrame(brush->layer(), brush->frame(), false);
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
		if(!object.contains("message") || !object.value("message").isString()) return;

		const QString name = object["name"].toString(), message = object["message"].toString();
		if(!message.isEmpty()){
			emit onChatMessage(message, name);
		}

		int id = object["id"].toInt();
		if(id == this->me) return;

		if(brushes.find(id) == brushes.end()) return;
		ParupaintVisualCursor * brush = brushes.value(id);
		if(brush && brush->status() == ParupaintVisualCursorStatus::CursorStatusTyping){
			brush->setStatus(ParupaintVisualCursorStatus::CursorStatusNone, 0);
			brush->update();
		}

	} else {
		//qDebug() << id << object;
	}
}
