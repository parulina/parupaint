#include "parupaintServerInstance.h"
// This file handles most network events
#include "parupaintConnection.h"

#include "../core/parupaintRecordPlayer.h"
#include "../core/parupaintRecordManager.h"
#include "../core/parupaintBrush.h"

#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintFrameBrushOps.h"
#include "../core/parupaintPanvasInputOutput.h"
#include "../core/parupaintCommonOperations.h"

#include "../parupaintVersion.h"

// toColor(hex)
#include "../core/parupaintSnippets.h"
#include "../bundled/qcompressor.h"

#include <QDir>
#include <QSettings>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

void ParupaintServerInstance::message(ParupaintConnection * c, const QString & id, const QByteArray & bytes)
{
	QJsonObject obj = QJsonDocument::fromJson(bytes).object();
	if(!c) return;
	obj["id"] = c->id();

	if(id == "connect"){
		c->setId(connectid++);
		foreach(ParupaintConnection * con, brushes.keys()){
			QJsonObject obj = this->connectionObj(con);
			obj["exists"] = true;
			c->send("brush", obj);
		}
		c->send("canvas", this->canvasObj());
		this->sendInfo();

		emit onConnect(c);

		// TODO replace with variable
		// TODO put brush immediately here? and make join message later when name
		QSettings cfg("server.ini", QSettings::IniFormat, this);
		if(cfg.value("autojoin", false).toBool()){
			c->setAutoJoinFlag(true);
		}
	}
	if(id == "join"){
		if(brushes.find(c) != brushes.end()) return;

		bool ok;
		qreal ver = obj["version"].toString("0.00").toDouble(&ok);
		if(!ok) return;

		if(ver < 0.92) return; // 0.91 and previous autojoins
		if(ver < 1.00 && ver != QString(PARUPAINT_VERSION).toDouble()) return;

		if(c->socket()->host() != "localhost"){
			if(!this->password().isEmpty()){
				QJsonObject msgobj;
				if(!obj["password"].isString()){
					msgobj["message"] = "Server requires a password.";
					c->send("chat", msgobj);
					return;
				}

				QString pw = obj["password"].toString();
				if(pw != this->password()){
					msgobj["message"] = "Incorrect password.";
					c->send("chat", msgobj);
					return;
				}
			}
		}
		this->joinConnection(c);
		this->sendInfo();
	}

	if(id == "name") {
		if(!obj["name"].isString()) return;

		QString name = obj["name"].toString();
		if(name.length() > 24 || name.isEmpty()) return;

		bool new_name = (!name.isEmpty() && c->name().isEmpty());
		c->setName(name);

		if(new_name && c->autoJoinFlag() && brushes.find(c) == brushes.end()){
			c->setAutoJoinFlag(false);
			this->joinConnection(c);
			this->sendInfo();
		} else {
			this->objMessage("brush", {
				{"id", obj["id"]},
				{"n", obj["name"]}
			});
		}
	}
	if(id == "leave" || id == "disconnect") {
		ParupaintBrush * brush = brushes.value(c);
		if(brush){
			this->leaveConnection(c);
		}

		if(id == "leave"){
			c->send("leave");
			emit onLeave(c);
		} else if(id == "disconnect"){
			emit onDisconnect(c);
		}
		this->sendInfo();
	}
	if(id == "chat") {
		if(!obj.contains("name")){
			obj["name"] = c->name();
		}
	}
	if(id == "canvas") {
		c->send(id, this->canvasObj());
	}
	if (id == "image") {
		// TODO params to specify which image
		for(int l = 0; l < canvas->layerCount(); l++){
			ParupaintLayer * layer = canvas->layerAt(l);
			for(int f = 0; f < layer->frameCount(); f++){
				ParupaintFrame * frame = layer->frameAt(f);
				if(!layer->isFrameReal(f)) continue;

				const QImage img = frame->image();

				const QByteArray fdata((const char*)img.bits(), img.byteCount());
				QByteArray compressed_bytes;
				QCompressor::gzipCompress(fdata, compressed_bytes);

				QJsonObject obj;
				obj["data"] = QString(compressed_bytes.toBase64());
				obj["w"] = img.width();
				obj["h"] = img.height();
				obj["l"] = l;
				obj["f"] = f;
				c->send(id, obj);
			}
		}
	}
	// do not do anything else if not joined.
	if(brushes.find(c) == brushes.end()) return;

	this->doMessage(id, obj);
}


void ParupaintServerInstance::doMessage(const QString & id, QJsonObject obj)
{
	if(id == "chat") {
		if(!obj.contains("name") || !obj.value("name").isString()){
			ParupaintConnection * con = this->getConnection(obj["id"].toInt(-1));
			if(con){
				obj["name"] = con->name();
			}
		}
		this->objMessage(id, {
			{"id", obj["id"]},
			{"name", obj["name"]},
			{"message", obj["message"]}
		});
	}
	if(id == "lfa") {
		if(!obj["l"].isDouble()) return;
		if(!obj["f"].isDouble()) return;
		if(!obj["attr"].isObject()) return;

		int l = obj["l"].toInt(), f = obj["f"].toInt();
		QJsonObject attr = obj["attr"].toObject();

		if(!attr.length()) return;
		foreach(const QString & key, attr.keys()){
			const QVariant & val = attr[key].toVariant();

			if(ParupaintCommonOperations::LayerFrameAttributeOp(canvas, l, f, key, val)){
				this->objMessage(id, {
					{"l", l}, {"f", f},
					{"attr", QJsonObject{ {key, attr[key]} }}
				});
			}
		}
	}

	if(id == "lfc") {
		if(!obj["l"].isDouble()) return;
		if(!obj["f"].isDouble()) return;
		if(!obj["lc"].isDouble()) return;
		if(!obj["fc"].isDouble()) return;

		int l = obj["l"].toInt(), f = obj["f"].toInt();
		int lc = obj["lc"].toInt(), fc = obj["fc"].toInt();
		bool ext = obj["ext"].toBool(false);

		if(ParupaintCommonOperations::LayerFrameChangeOp(canvas, l, f, lc, fc, ext)){
			foreach(ParupaintBrush * brush, this->brushes){
				if(l <= brush->layer()) brush->setLayer(brush->layer() + lc);
				if(f <= brush->frame()) brush->setFrame(brush->frame() + fc);
				ParupaintCommonOperations::AdjustBrush(brush, canvas);
			}
			this->objMessage(id, {
				{"l", obj["l"]}, {"f", obj["f"]},
				{"lc", obj["lc"]}, {"fc", obj["fc"]},
				{"ext", obj["ext"]}
			});
		}

	}
	if(id == "fill"){
		if(!obj["l"].isDouble()) return;
		if(!obj["f"].isDouble()) return;
		if(!obj["c"].isString()) return;

		int l = obj["l"].toInt(), f = obj["f"].toInt();
		QString c = obj["c"].toString();

		if(ParupaintCommonOperations::LayerFrameFillOp(canvas, l, f, QColor(c))){
			this->objMessage(id, {
				{"l", obj["l"]}, {"f", obj["f"]},
				{"c", obj["c"]}
			});
		}

	}
	if(id == "new") {
		if(!obj["w"].isDouble()) return;
		if(!obj["h"].isDouble()) return;
		if(!obj["r"].isBool()) return;

		bool r = obj["r"].toBool();
		int w = obj["w"].toInt(),
		    h = obj["h"].toInt();

		if(ParupaintCommonOperations::CanvasResizeOp(canvas, w, h, r)){
			foreach(ParupaintBrush * brush, this->brushes){
				ParupaintCommonOperations::AdjustBrush(brush, canvas);
			}
			this->objMessage(id, {
				{"w", obj["w"]}, {"h", obj["h"]},
				{"r", obj["r"]},
			});
		}
	}
	if(id == "paste"){
		if(!obj["image"].isString()) return;
		if(!obj["l"].isDouble()) return;
		if(!obj["f"].isDouble()) return;

		QString image = obj["image"].toString();
		QImage img = ParupaintSnippets::Base64GzipToImage(image);

		int l = obj["l"].toInt(0),
		    f = obj["f"].toInt(0),
		    x = obj["x"].toInt(0),
		    y = obj["y"].toInt(0);

		if(ParupaintCommonOperations::LayerFramePasteOp(canvas, l, f, x, y, img)){
			this->objMessage(id, {
				{"l", obj["l"]}, {"f", obj["f"]},
				{"x", obj["x"]}, {"y", obj["y"]},
				{"image", obj["image"]},
			});
		}

	}
	if(id == "info") {
		foreach(const QString & key, obj.keys()){
			QVariant val = obj[key].toVariant();
			if(key == "sessionpw"){
				this->setPassword(obj.take("sessionpw").toString());
				continue;
			}
			if(key == "project-bgc" && val.type() == QVariant::String)
				val = QColor(val.toString());

			ParupaintCommonOperations::CanvasAttributeOp(canvas, key, val);
		}
		this->objMessage(id, obj);
	}

	if(id == "brush"){
		if(!obj["id"].isDouble()) return;

		int i = obj["id"].toInt();
		ParupaintConnection * con = this->getConnection(i);
		ParupaintBrush * brush = this->brushes.value(con);
		if(brush) {

			QLineF draw_line;
			double old_pressure = brush->pressure();
			double old_size = brush->pressureSize();
			QPoint old_pos = brush->pixelPosition();

			// convert json stuff to map
			QVariantMap map = {{"id", i}};
			if(obj["st"].isDouble()) map["st"] = obj["st"].toInt();

			if(obj["x"].isDouble()) map["x"] = obj["x"].toDouble();
			if(obj["y"].isDouble()) map["y"] = obj["y"].toDouble();
			if(obj["p"].isDouble()) map["p"] = obj["p"].toDouble();
			if(obj["d"].isBool())   map["d"] = obj["d"].toBool();

			if(obj["c"].isString()) { map["c"] = QColor(obj["c"].toString());
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"c", obj["c"]} });
			}
			if(obj["s"].isDouble()) { map["s"] = obj["s"].toDouble();
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"s", obj["s"]} });
			}
			if(obj["t"].isDouble()) { map["t"] = obj["t"].toInt();
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"t", obj["t"]} });
			}
			if(obj["l"].isDouble()) { map["l"] = obj["l"].toInt();
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"l", obj["l"]} });
			}
			if(obj["f"].isDouble()) { map["f"] = obj["f"].toInt();
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"f", obj["f"]} });
			}


			// assign the stuff
			ParupaintCommonOperations::BrushOp(brush, draw_line, map);
			if(map.contains("d"))     record_manager.writeLogFile(id, { {"id", obj["id"]}, {"d", obj["d"]}, {"x", brush->x()}, {"y", brush->y()} });

			if(brush->drawing()){
				ParupaintFrameBrushOps::stroke(canvas, brush, draw_line, old_size);
			}
			if(brush->tool() == ParupaintBrushToolTypes::BrushToolFloodFill) brush->setDrawing(false);


			// write new pos
			if(brush->drawing() && (old_pos != brush->pixelPosition() || old_pressure != brush->pressure())) {
				QJsonObject o{
					{"id", obj["id"]}
				};
				if(brush->pixelPosition().x() != old_pos.x()) o["x"] = brush->pixelPosition().x();
				if(brush->pixelPosition().y() != old_pos.y()) o["y"] = brush->pixelPosition().y();
				if(brush->pressure() != old_pressure) o["p"] = brush->pressure();
				record_manager.writeLogFile(id, o);
			}
			// send update
			this->sendAll(id, obj, con);
		}
	}
	/*
	if(id == "save") {
		if(brushes.find(c) == brushes.end()) return;

		// TODO clean this up
		QSettings cfg;
		QString name = obj["filename"].toString();
		QString load_dir = cfg.value("canvas/directory", ".").toString();

		if(!name.isEmpty()){
			QString err, fname(load_dir + "/" + name);
			bool ret = false;
			if((ret = ParupaintPanvasInputOutput::savePanvas(canvas, fname, err))){
				QString msg = QString("Server saved canvas successfully at: \"%1\"").arg(name);
				this->sendChat(msg);
			} else {
				QString msg("Failed saving canvas: " + err);
				this->sendChat(msg);
			}
		}

	} else if(id == "load") {
		if(brushes.find(c) == brushes.end()) return;

		QSettings cfg;
		QString load_dir = cfg.value("server/directory", ".").toString();

		// TODO clean this up
		if(obj["file"].isString() && obj["filename"].isString()){

			// a gzipped base64 encoded file is sent, uncompress it and write to temp file
			QByteArray data = QByteArray::fromBase64(obj["file"].toString().toUtf8());
			QByteArray uncompressed;
			QCompressor::gzipDecompress(data, uncompressed);

			obj["filename"] = "temp_load" + QFileInfo(obj["filename"].toString()).suffix();

			load_dir = QDir::temp().path();
			QFileInfo ff(load_dir, obj["filename"].toString());

			QFile file(ff.filePath());
			if(!file.open(QIODevice::WriteOnly)){
				obj["filename"] = QJsonValue::Undefined;
				return;
			}
			file.write(uncompressed);
			file.close();
		}

		if(obj["filename"].isString()){
			QString name = obj["filename"].toString();
			if(name.isEmpty()) return;

			QString err, fname(load_dir + "/" + name);
			bool ret = false;
			if((ret = ParupaintPanvasInputOutput::loadPanvas(canvas, fname, err))){
				this->sendChat("Server loaded file successfully!");
				this->sendAll("canvas", this->canvasObj());
			} else {
				QString msg("Failed loading canvas: " + err);
				this->sendChat(msg);
			}
		}
	*/
	emit OnMessage(id, obj);
}

