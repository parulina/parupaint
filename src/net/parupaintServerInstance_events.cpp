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
#include <QTemporaryFile>
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
	if(id == "play") {
		if(brushes.find(c) == brushes.end()) return;
		if(!obj.contains("filename") || !obj.value("filename").isString()) return;

		QString name = obj["filename"].toString();
		int limit = obj.value("limit").toInt(-1);
		if(!name.isEmpty()){

			QFileInfo file_load(server_dir, name);
			qDebug() << "Playback log" << file_load.absolutePath() << "with limit" << limit;

			if(file_load.exists()){
				this->playLogFile(file_load.absoluteFilePath(), limit);
			}
		}
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
				if(l-1 <= brush->layer()) brush->setLayer(brush->layer() + lc);
				if(f-1 <= brush->frame()) brush->setFrame(brush->frame() + fc);
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

			if(!r) this->resetCanvasLog();

			QJsonObject canvas = this->canvasObj();
			canvas["resize"] = obj["r"];
			this->sendAll("canvas", canvas);
		}
	}
	if(id == "paste"){
		if(!obj["image"].isString()) return;
		if(!obj["l"].isDouble()) return;
		if(!obj["f"].isDouble()) return;

		QString image = obj["image"].toString();
		QImage img = ParupaintSnippets::Base64GzipToImage(image);

		if(img.size().width() > canvas->dimensions().width() ||
		   img.size().height() > canvas->dimensions().height()) return;

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
			if(obj.contains("st") && obj["st"].isDouble()) map["st"] = obj["st"].toInt();

			if(obj.contains("x") && obj["x"].isDouble()) map["x"] = obj["x"].toDouble();
			if(obj.contains("y") && obj["y"].isDouble()) map["y"] = obj["y"].toDouble();
			if(obj.contains("p") && obj["p"].isDouble()) map["p"] = obj["p"].toDouble();
			if(obj.contains("d") && obj["d"].isBool())   map["d"] = obj["d"].toBool();

			if(obj.contains("c") && obj["c"].isString()) map["c"] = QColor(obj["c"].toString());
			if(obj.contains("s") && obj["s"].isDouble()) map["s"] = obj["s"].toDouble();

			if(obj.contains("t") && obj["t"].isDouble()) map["t"] = obj["t"].toInt();
			if(obj.contains("a") && obj["a"].isDouble()) map["a"] = obj["a"].toInt();
			if(obj.contains("l") && obj["l"].isDouble()) map["l"] = obj["l"].toInt();
			if(obj.contains("f") && obj["f"].isDouble()) map["f"] = obj["f"].toInt();


			// assign the stuff
			ParupaintCommonOperations::BrushOp(brush, draw_line, map);
			if(map.contains("d")) {
				record_manager.writeLogFile(id, { {"id", obj["id"]}, {"d", obj["d"]}, {"x", brush->x()}, {"y", brush->y()} });
			}

			// could possibly be done a lot smoother...
			if(map.contains("c")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"c", map["c"].value<QColor>().name(QColor::HexArgb)} });
			if(map.contains("s")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"s", map["s"].toInt()} });
			if(map.contains("t")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"t", map["t"].toInt()} });
			if(map.contains("a")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"a", map["a"].toInt()} });
			if(map.contains("l")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"l", map["l"].toInt()} });
			if(map.contains("f")) record_manager.writeLogFile(id, { {"id", map["id"].toInt()}, {"f", map["f"].toInt()} });


			if(brush->drawing()){
				ParupaintFrameBrushOps::stroke(canvas, brush, draw_line, old_size);
			}
			if(brush->tool() == ParupaintBrushTool::BrushToolFloodFill) brush->setDrawing(false);


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
	if(id == "save") {
		if(!obj.contains("filename") || !obj.value("filename").isString()) return;

		const QString name = obj["filename"].toString();
		if(!name.isEmpty()){
			QString err, fname(server_dir.path() + "/" + name);
			bool ret = false;
			if((ret = ParupaintPanvasInputOutput::savePanvas(canvas, fname, err))){
				QString msg = QString("Server saved canvas successfully at: \"%1\"").arg(name);
				this->sendChat(msg);
			} else {
				QString msg("Failed saving canvas: " + err);
				this->sendChat(msg);
			}
		}
	}
	if(id == "load") {
		if(!obj.contains("filename") || !obj.value("filename").isString()) return;

		QString name = obj["filename"].toString();
		if(!name.isEmpty()){
			QTemporaryFile temp_file;
			QFileInfo file_load(server_dir, name);

			// if there's a file embedded, then name should contain the extension
			if(obj.contains("file") && obj.value("file").isString() && name.indexOf('.') == -1){

				// base64 gzip file
				QByteArray data = QByteArray::fromBase64(obj["file"].toString().toUtf8());
				QByteArray uncompressed;
				QCompressor::gzipDecompress(data, uncompressed);

				temp_file.setFileTemplate(server_dir.path() + "/.load.XXXXXX." + name);
				if(temp_file.open()){
					temp_file.write(uncompressed);
					temp_file.close();

					file_load.setFile(temp_file.fileName());
				}

			}
			QString err, fname = file_load.absoluteFilePath();
			qDebug() << "Load file" << fname;
			bool ret = false;
			if((ret = ParupaintPanvasInputOutput::loadPanvas(canvas, fname, err))){
				this->sendChat("Server loaded file successfully!");
				this->sendAll("canvas", this->canvasObj());
			} else {
				QString msg("Failed loading canvas: " + err);
				this->sendChat(msg);
			}
		}
	}
	emit OnMessage(id, obj);
}

