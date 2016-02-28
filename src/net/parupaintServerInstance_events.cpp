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

// Makes someone join the server.
// a brush is created.
void ParupaintServerInstance::ServerJoin(ParupaintConnection * c, bool propagate)
{
	if(!c) return;
	if(!brushes[c]){
		brushes[c] = new ParupaintBrush();
	}

	if(record_manager) record_manager->Join(c->id());

	if(!propagate) return;

	QJsonObject obj = this->connectionObj(c);
	obj["exists"] = true;
	this->sendAll("brush", obj, c);
}

void ParupaintServerInstance::ServerLeave(ParupaintConnection * c, bool propagate)
{
	if(!c) return;
	if(brushes.find(c) == brushes.end()) return;
	delete brushes[c];
	brushes.remove(c);

	if(record_manager) record_manager->Leave(c->id());

	if(!propagate) return;

	QJsonObject obj = this->connectionObj(c);
	obj["exists"] = false;
	this->sendAll("brush", obj, c);
}
// set the name!
void ParupaintServerInstance::ServerName(ParupaintConnection * c, QString name, bool propagate)
{
	if(!c) return;

	c->setName(name);

	ParupaintBrush * brush = brushes.value(c);
	if(brush){
		brush->setName(name);
	}

	if(record_manager) record_manager->Name(c->id(), name);

	if(!propagate) return;

	if(brush){
		QJsonObject obj;
		obj["n"] = name;
		obj["id"] = c->id();
		this->sendAll("brush", obj, c);
	}
}
void ParupaintServerInstance::ServerChat(ParupaintConnection * c, QString msg, bool propagate)
{
	if(record_manager) record_manager->Chat(c->id(), msg);

	if(!propagate) return;

	QJsonObject obj;
	obj["message"] = msg;
	obj["name"] = c->name();
	obj["id"] = c->id();
	this->sendAll("chat", obj);
}


void ParupaintServerInstance::ServerLfa(int l, int f, const QString & attr, const QVariant & val, bool propagate)
{
	if(!ParupaintCommonOperations::LayerFrameAttributeOp(canvas, l, f, attr, val)) return;
	if(record_manager) record_manager->Lfa(l, f, attr, val);

	if(!propagate) return;

	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["attr"] = QJsonObject{
		{attr, QJsonValue::fromVariant(val)}
	};
	this->sendAll("lfa", obj);
}
void ParupaintServerInstance::ServerLfc(int l, int f, int lc, int fc, bool e, bool propagate)
{
	bool changed = false;
	if(!(changed = ParupaintCommonOperations::LayerFrameChangeOp(canvas, l, f, lc, fc, e))) return;

	foreach(ParupaintBrush * brush, this->brushes){
		if(l <= brush->layer()) brush->setLayer(brush->layer() + lc);
		if(f <= brush->frame()) brush->setFrame(brush->frame() + fc);
		ParupaintCommonOperations::AdjustBrush(brush, canvas);
	}

	if(record_manager) record_manager->Lfc(l, f, lc, fc, e);

	if(!propagate) return;

	if((lc != 0 || fc != 0) && changed){
		QJsonObject obj;
		obj["l"] = l;
		obj["f"] = f;
		obj["lc"] = lc;
		obj["fc"] = fc;
		obj["ext"] = e;
		this->sendAll("lfc", obj);
	}

}
void ParupaintServerInstance::ServerFill(int l, int f, QString fill, bool propagate)
{
	QColor col(fill);
	if(!ParupaintCommonOperations::LayerFrameFillOp(canvas, l, f, col)) return;
	if(record_manager) record_manager->Fill(l, f, fill);

	if(!propagate) return;
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = fill;
	this->sendAll("fill", obj);
}

void ParupaintServerInstance::ServerPaste(int l, int f, int x, int y, QString base64_img, bool propagate)
{ this->ServerPaste(l, f, x, y, ParupaintSnippets::Base64GzipToImage(base64_img), propagate); }

void ParupaintServerInstance::ServerPaste(int l, int f, int x, int y, QImage img, bool propagate)
{
	if(!ParupaintCommonOperations::LayerFramePasteOp(canvas, l, f, x, y, img)) return;

	QString base64_img = ParupaintSnippets::ImageToBase64Gzip(img);
	if(record_manager) record_manager->Paste(l, f, x, y, base64_img);

	if(!propagate) return;
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["x"] = x;
	obj["y"] = y;
	obj["paste"] = base64_img;
	this->sendAll("paste", obj);
}

void ParupaintServerInstance::ServerResize(int w, int h, bool r, bool propagate)
{
	if(!ParupaintCommonOperations::CanvasResizeOp(canvas, w, h, r)) return;

	foreach(ParupaintBrush * brush, this->brushes){
		ParupaintCommonOperations::AdjustBrush(brush, canvas);
	}

	if(record_manager) record_manager->Resize(w, h, r);

	if(!propagate) return;
	QJsonObject obj;
	obj["w"] = w;
	obj["h"] = h;
	obj["r"] = r;
	this->sendAll("new", obj);
}



void ParupaintServerInstance::message(ParupaintConnection * c, const QString & id, const QByteArray & bytes)
{
	const QJsonObject obj = QJsonDocument::fromJson(bytes).object();
	QJsonObject obj_copy = obj;

	if(c) {
		if(id == "connect"){
			c->setId(connectid++);
			// send everyone and the canvas too
			foreach(ParupaintConnection * con, brushes.keys()){
				if(con == c) continue;
				QJsonObject obj = this->connectionObj(con);
				obj["exists"] = true;
				c->send("brush", obj);
			}
			c->send("canvas", this->canvasObj());

			QJsonObject pw_obj;
			pw_obj["password"] = !this->password().isEmpty();
			c->send("info", pw_obj);

			emit onConnect(c);

			QSettings cfg("server.ini", QSettings::IniFormat, this);
			if(cfg.value("autojoin", false).toBool()){
				c->setAutoJoinFlag(true);
			}
		} else if(id == "join"){
			// don't join again.
			if(brushes.find(c) != brushes.end()) return;

			bool ok;
			QJsonObject msgobj;
			qreal ver = obj["version"].toString("0.00").toDouble(&ok);

			if(!ok) return;
			if(ver < 0.92) return; // 0.91 and previous autojoins

			// parupaint will probably update a lot before the official release.
			if(ver < 1.00 && ver != QString(PARUPAINT_VERSION).toDouble()) return;

			if(c->socket()->host() != "localhost"){
				if(!this->password().isEmpty()){
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

		// happens even if client doesn't send leave message
		} else if(id == "leave" || id == "disconnect") {
			ParupaintBrush * brush = brushes.value(c);
			if(brush){
				this->ServerLeave(c, true);
			}

			if(id == "leave"){
				c->send("leave");
				emit onLeave(c);
			} else if(id == "disconnect"){
				emit onDisconnect(c);
			}

		} else if(id == "name") {
			if(!obj["name"].isString()) return;
			QString name = obj["name"].toString();
			bool new_name = (!name.isEmpty() && c->name().isEmpty());

			// name is its own packet because a connection
			// can have a name even without a brush
			// so it wouldn't go in the brush packet
			if(name.length() > 24) return;
			this->ServerName(c, name, true);

			if(new_name && c->autoJoinFlag()){
				c->setAutoJoinFlag(false);
				this->joinConnection(c);
			}

		} else if(id == "lfa") {
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;
			if(!obj["attr"].isObject()) return;

			int l = obj["l"].toInt(), f = obj["f"].toInt();
			QJsonObject attr = obj["attr"].toObject();

			if(!attr.length()) return;
			// eh, it's not worth it to make it work only with one key
			foreach(const QString & key, attr.keys()){
				const QVariant & val = attr[key].toVariant();

				this->ServerLfa(l, f, key, val);
			}
		} else if(id == "lfc") {
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;
			if(!obj["lc"].isDouble()) return;
			if(!obj["fc"].isDouble()) return;

			this->ServerLfc(obj["l"].toInt(), obj["f"].toInt(),
					obj["lc"].toInt(), obj["fc"].toInt(),
					obj["ext"].toBool(false));

		} else if(id == "fill"){
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;
			if(!obj["c"].isString()) return;
			int l = obj["l"].toInt(),
			    f = obj["f"].toInt();
			QString c = obj["c"].toString();

			this->ServerFill(l, f, c);

		} else if(id == "brush"){
			ParupaintBrush * brush = brushes.value(c);
			if(brush) {

				QLineF draw_line;
				double old_size = brush->pressureSize();

				// convert json stuff to map
				QVariantMap map;
				if(obj["x"].isDouble()) map["x"] = obj["x"].toDouble();
				if(obj["y"].isDouble()) map["y"] = obj["y"].toDouble();
				if(obj["p"].isDouble()) map["p"] = obj["p"].toDouble();
				if(obj["l"].isDouble()) map["l"] = obj["l"].toInt();
				if(obj["f"].isDouble()) map["f"] = obj["f"].toInt();
				if(obj["d"].isBool())   map["d"] = obj["d"].toBool();

				if(obj["c"].isString()) { map["c"] = QColor(obj["c"].toString());
					if(record_manager) record_manager->Color(c->id(), obj["c"].toString());
				}
				if(obj["s"].isDouble()) { map["s"] = obj["s"].toDouble();
					if(record_manager) record_manager->Width(c->id(), obj["s"].toDouble());
				}
				if(obj["t"].isDouble()) { map["t"] = obj["t"].toInt();
					if(record_manager) record_manager->Tool(c->id(), obj["t"].toInt());
				}

				// assign the stuff
				ParupaintCommonOperations::BrushOp(brush, draw_line, map);
				if(brush->drawing()){
					ParupaintFrameBrushOps::stroke(canvas, brush, draw_line, old_size);
				}
				if(brush->tool() == ParupaintBrushToolTypes::BrushToolFloodFill) brush->setDrawing(false);


				// if l or f was changed
				if(obj["l"].isDouble() || obj["f"].isDouble()){
					if(record_manager) record_manager->Lf(c->id(), brush->layer(), brush->frame());
				}
				// draw, x, y, or pressure was changed
				if(obj["d"].isBool() == brush->drawing() || obj["x"].isDouble() || obj["y"].isDouble() || obj["p"].isDouble()) {
					if(record_manager) record_manager->Pos(c->id(), brush->x(), brush->y(), brush->pressure(), brush->drawing());
				}

				obj_copy["id"] = c->id();
				this->sendAll(id, obj_copy, c);
			}
		} else if(id == "canvas") {
			c->send(id, this->canvasObj());

		} else if (id == "image") {

			for(auto l = 0; l < canvas->layerCount(); l++){
				auto * layer = canvas->layerAt(l);
				for(auto f = 0; f < layer->frameCount(); f++){
					auto * frame = layer->frameAt(f);
					if(!layer->isFrameReal(f)) continue;

					const auto img = frame->image();
					
 					const QByteArray fdata((const char*)img.bits(), img.byteCount());
					QByteArray compressed_bytes;
					QCompressor::gzipCompress(fdata, compressed_bytes);

					QJsonObject obj;
					obj["data"] = QString(compressed_bytes.toBase64());
					obj["w"] = img.width();
					obj["h"] = img.height();
					obj["l"] = l;
					obj["f"] = f;
					c->send(id, QJsonDocument(obj).toJson(QJsonDocument::Compact));
				}
			}
		} else if(id == "new") {
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["w"].isDouble()) return;
			if(!obj["h"].isDouble()) return;
			if(!obj["r"].isBool()) return;

			bool r = obj["r"].toBool();
			int w = obj["w"].toInt(),
			    h = obj["h"].toInt();

			this->ServerResize(w, h, r);

		} else if(id == "paste"){
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["image"].isString()) return;
			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;

			QString image = obj["image"].toString();
			int l = obj["l"].toInt(0),
			    f = obj["f"].toInt(0),
			    x = obj["x"].toInt(0),
			    y = obj["y"].toInt(0);
			this->ServerPaste(l, f, x, y, image);

		} else if(id == "chat") {
			if(obj["message"].isString()){
				QString msg = obj["message"].toString();
				this->ServerChat(c, obj["message"].toString());
			} else {
				QJsonObject obj;
				obj["id"] = c->id();
				this->sendAll("chat", obj, c);
			}

		} else if(id == "save") {
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
		} else if(id == "info") {
			if(brushes.find(c) == brushes.end()) return;

			foreach(const QString & key, obj.keys()){
				QVariant val = obj[key].toVariant();
				if(key == "sessionpw"){
					this->setPassword(val.toString());
					obj_copy.remove("sessionpw");
					continue;
				}
				if(key == "project-bgc" && val.type() == QVariant::String) val = QColor(val.toString());
				ParupaintCommonOperations::CanvasAttributeOp(canvas, key, val);
			}
			// send obj_copy so that password isn't sent to everyone
			this->sendAll("info", obj_copy);


		} else if(id == "play") {
			if(brushes.find(c) == brushes.end()) return;

			if(!obj["filename"].isString()) return;
			if(!obj["as_script"].isBool()) return;
			QString filename = obj["filename"].toString();
			bool as_script = obj["as_script"].toBool();

			QFile record_file(filename);
			if(record_file.open(QFile::ReadOnly)){
				record_player->LoadFromFile(record_file);
				record_file.close();

				QJsonObject obj;
				obj["count"] = record_player->GetTotalLines();
				this->sendAll("play", obj);

				this->SaveRecordBrushes();
				if(as_script){
					QString line;

					while(!record_player->TakeLine(line)){
						this->RecordLineDecoder(line, false);
					}
					// play signal is still required here so that
					// changes to real brushes stay...
					if(record_player->WillRestore()) this->RestoreRecordBrushes();
					record_player->Reset();

					QJsonObject obj;
					obj["count"] = 0;
					this->sendAll("play", obj);

				} else {
					this->StartRecordTimer();
				}
			}
		} else {
			//qDebug() << id << obj;
		}
	}
	// put at the end because some messages
	// are return; ing because error/access level/etc
	// it only reaches this point if everything went well
	emit OnMessage(id, obj);
}

