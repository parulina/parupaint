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

// toColor(hex)
#include "../core/parupaintSnippets.h"
#include "../bundled/qcompressor.h"

#include <QDir>
#include <QSettings>
#include <QJsonObject>
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

	QJsonObject obj_me = this->MarshalConnection(c);
	obj_me["disconnect"] = false;
	this->sendAll("peer", obj_me, c);
}

void ParupaintServerInstance::ServerLeave(ParupaintConnection * c, bool propagate)
{
	if(!c) return;
	if(brushes.find(c) == brushes.end()) return;
	delete brushes[c];
	brushes.remove(c);

	if(record_manager) record_manager->Leave(c->id());

	if(!propagate) return;

	QJsonObject obj;
	obj["disconnect"] = true;
	obj["id"] = c->id();
	this->sendAll("peer", obj, c);
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
		obj["name"] = name;
		obj["id"] = c->id();
		this->sendAll("name", obj, c);
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
void ParupaintServerInstance::ServerLfc(int l, int f, int lc, int fc, bool e, bool propagate)
{
	bool changed = false;
	if(lc != 0){
		if(lc < 0 && canvas->layerCount() > 1){
			for(int i = 0; i < -lc; i++){
				canvas->removeLayer(l);
			}
			changed = true;
		} else if(lc > 0){
			for(int i = 0; i < lc; i++){
				canvas->insertLayer(l);
			}
			changed = true;
		}
	}
	if(fc != 0){
		auto * ff = canvas->layerAt(l);
		if(ff) {
			if(fc < 0 && ff->frameCount() > 1){
				if(e){
					for(int i = 0; i < -fc; i++)
						ff->redactFrame(f);
				} else {
					for(int i = 0; i < -fc; i++)
						ff->removeFrame(f);
				}
				changed = true;
			} else if(fc > 0){
				if(e){
					for(int i = 0; i < fc; i++)
						ff->extendFrame(f);
				} else {
					for(int i = 0; i < fc; i++)
						ff->insertFrame(canvas->dimensions(), f);
				}
				changed = true;
			}
		}
	}

	if(record_manager) record_manager->Lfc(l, f, lc, fc, e);

	// TODO loop through every brush and reset their pos
	if(!propagate) return;
	// TODO and make sure they update their values too

	if((lc != 0 || fc != 0) && changed){
		this->sendAll("canvas", this->canvasObj());
	}

}
void ParupaintServerInstance::ServerFill(int l, int f, QString fill, bool propagate)
{
	QColor col = ParupaintSnippets::toColor(fill);

	ParupaintLayer * layer = canvas->layerAt(l);
	if(layer){
		ParupaintFrame * frame = layer->frameAt(f);
		if(frame){
			frame->clear(col);
		}
	}
	if(record_manager) record_manager->Fill(l, f, fill);

	if(!propagate) return;
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = fill;
	this->sendAll("fill", obj);
}

void ParupaintServerInstance::ServerPaste(int l, int f, int x, int y, QImage img, bool propagate)
{
	this->ServerPaste(l, f, x, y, ParupaintSnippets::ImageToBase64Gzip(img), propagate);
}
void ParupaintServerInstance::ServerPaste(int l, int f, int x, int y, QString base64_img, bool propagate)
{
	// paste has to be a base64 thing.
	QImage img = ParupaintSnippets::Base64GzipToImage(base64_img);
	if(!img.isNull()){
		ParupaintLayer * layer = canvas->layerAt(l);
		if(layer){
			ParupaintFrame * frame = layer->frameAt(f);
			if(frame){
				frame->drawImage(QPointF(x, y), img);
			}
		}
	}
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
	if(!r){
		canvas->clearCanvas();
		canvas->insertLayer(0, 1);
	}
	// because resizing only one frame is the quickest.
	canvas->resize(QSize(w, h));
	if(record_manager) record_manager->Resize(w, h, r);

	if(!propagate) return;
	this->sendAll("canvas", this->canvasObj());
}

void ParupaintServerInstance::message(ParupaintConnection * c, const QString & id, const QByteArray & bytes)
{
	const QJsonObject obj = QJsonDocument::fromJson(bytes).object();
	QJsonObject obj_copy = obj;

	if(c) {
		if(id == "connect"){
			// send everyone and the canvas too
			foreach(ParupaintConnection * con, brushes.keys()){
				c->send("peer", this->MarshalConnection(con));
			}
			c->send("canvas", this->canvasObj());

		// disconnect is handled lower down
		// } else if(id == "disconnect"){
			// connect and disconnect is for stuff that
			// should always happen to joining people

		} else if(id == "join"){
			bool ok;
			QJsonObject msgobj;
			qreal ver = obj["version"].toString("0.00").toDouble(&ok);

			if(!ok) return;
			if(ver < 0.92) return; // 0.91 and previous autojoins

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

			// let the connection know we accepted them
			c->send("join");

			c->setId(connectid++);
			this->ServerJoin(c, true);

		// happens even if client doesn't send leave message
		} else if(id == "leave" || id == "disconnect") {
			if(id == "leave"){
				c->send("leave", "");
			}
			ParupaintBrush * brush = brushes.value(c);
			if(brush){
				this->ServerLeave(c, true);
			}

		} else if(id == "name") {
			if(!obj["name"].isString()) return;
			QString name = obj["name"].toString();

			if(name.length() > 24) return;
			this->ServerName(c, name, true);

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

		} else if(id == "draw"){
			ParupaintBrush * brush = brushes.value(c);
			if(brush) {
				double old_x = brush->x(),
				       old_y = brush->y();
				double x = old_x, y = old_y;

				// should come before false->true drawing to move brush
				// to correct pos
				if(obj["x"].isDouble()) x = obj["x"].toDouble();
				if(obj["y"].isDouble()) y = obj["y"].toDouble();

				if(obj["c"].isString()) {
					brush->setColor(ParupaintSnippets::toColor(obj["c"].toString()));
					if(record_manager) record_manager->Color(c->id(), obj["c"].toString());
				}
				if(obj["s"].isDouble()) {
					brush->setSize(obj["w"].toDouble());
					if(record_manager) record_manager->Width(c->id(), brush->size());
				}
				if(obj["t"].isDouble()) {
					brush->setTool(obj["t"].toInt());
					if(record_manager) record_manager->Tool(c->id(), brush->tool());
				}
				if(obj["p"].isDouble()) brush->setPressure(obj["p"].toDouble());
				if(obj["d"].isBool())	{
					const bool drawing = obj["d"].toBool();
					if(drawing && !brush->drawing()){
						if(record_manager) record_manager->Pos(c->id(), x, y, brush->pressure(), false);
						if(brush->tool() == ParupaintBrushToolTypes::BrushToolFloodFill){
							// ink click
							if(record_manager) record_manager->Pos(c->id(), x, y, 1, true);
						}
					}
					old_x = x; old_y = y;
					brush->setDrawing(drawing);
				}

				if(obj["l"].isDouble()) brush->setLayer(obj["l"].toInt());
				if(obj["f"].isDouble()) brush->setFrame(obj["f"].toInt());
				if(obj["l"].isDouble() || obj["f"].isDouble()) {
					if(record_manager) record_manager->Lf(c->id(), brush->layer(), brush->frame());
				}

				if(obj["x"].isDouble() && obj["y"].isDouble() && brush->drawing()) {
					if(record_manager) record_manager->Pos(c->id(), x, y, brush->pressure(), true);
				}

				if(brush->drawing()){
					ParupaintFrameBrushOps::stroke(canvas, brush, QPointF(x, y), QPointF(old_x, old_y));
					if(brush->tool() == ParupaintBrushToolTypes::BrushToolFloodFill){
						brush->setDrawing(false);
					}
				}

				brush->setPosition(QPointF(x, y));

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

			int width =  obj["w"].toInt(),
			    height = obj["h"].toInt();
			bool resize = obj["r"].toBool();

			if(width >= 8192 || height >= 8192) return;
			if(width <=    0 || height <=    0) return;

			this->ServerResize(width, height, resize);

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
			QString load_dir = cfg.value("canvas/directory").toString();

			if(!name.isEmpty()){
				QString err, fname(load_dir + "/" + name);
				bool ret = false;
				if((ret = ParupaintPanvasInputOutput::savePanvas(canvas, fname, err))){
					QString msg = QString("Server saved canvas successfully at: \"%1\"").arg(name);
					this->BroadcastChat(msg);
				} else {
					QString msg("Failed saving canvas: " + err);
					this->BroadcastChat(msg);
				}
			}

		} else if(id == "load") {
			if(brushes.find(c) == brushes.end()) return;

			QSettings cfg;
			QString load_dir = cfg.value("server/directory").toString();

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
					this->BroadcastChat("Server loaded file successfully!");
					this->sendAll("canvas", this->canvasObj());
				} else {
					QString msg("Failed loading canvas: " + err);
					this->BroadcastChat(msg);
				}
			}
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

