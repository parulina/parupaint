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
#include "../core/parupaintPanvasReader.h"
#include "../core/parupaintPanvasWriter.h"

// toColor(hex)
#include "../core/parupaintSnippets.h"
#include "../bundled/qcompressor.h"

#include <QDir>
#include <QSettings>
#include <QJsonObject>
#include <QJsonDocument>

// Makes someone join the server with a given name.
// a brush is created.
void ParupaintServerInstance::ServerJoin(ParupaintConnection * c, QString name, bool propagate)
{
	if(!c) return;
	if(!brushes[c]){
		brushes[c] = new ParupaintBrush();
	}

	brushes[c]->SetName(name);
	if(record_manager) record_manager->Join(c->getId(), name);

	if(!propagate) return;
	foreach(auto c2, brushes.keys()){

		QJsonObject obj_me = this->MarshalConnection(c);
		if(c2 == c) obj_me["id"] = -(c->getId());
		obj_me["disconnect"] = false;

		// send me to others
		c2->send("peer", obj_me);
		if(c2 == c) continue;

		// send them to me
		QJsonObject obj_them = this->MarshalConnection(c2);
		c->send("peer", obj_them);
	}
}

void ParupaintServerInstance::ServerLeave(ParupaintConnection * c, bool propagate)
{
	if(!c) return;
	if(brushes.find(c) == brushes.end()) return;
	delete brushes[c];
	brushes.remove(c);

	if(record_manager) record_manager->Leave(c->getId());

	if(!propagate) return;
	QJsonObject obj;
	obj["disconnect"] = true;
	obj["id"] = c->getId();
	this->Broadcast("peer", obj);
}
void ParupaintServerInstance::ServerChat(ParupaintConnection * c, QString msg, bool propagate)
{
	if(record_manager) record_manager->Chat(c->getId(), msg);

	if(!propagate) return;

	auto * brush = brushes.value(c);
	if(brush) {
		QJsonObject obj;
		obj["message"] = msg;
		obj["name"] = brush->GetName();
		obj["id"] = c->getId();
		this->Broadcast("chat", obj);
	}
}
void ParupaintServerInstance::ServerLfc(int l, int f, int lc, int fc, bool e, bool propagate)
{
	bool changed = false;
	if(lc != 0){
		if(lc < 0 && canvas->GetNumLayers() > 1){
			canvas->RemoveLayers(l, -lc);
			changed = true;
		} else if(lc > 0){
			canvas->AddLayers(l, lc, 1);
			changed = true;
		}
	}
	if(fc != 0){
		auto * ff = canvas->GetLayer(l);
		if(ff) {
			if(fc < 0 && ff->GetNumFrames() > 1){
				if(e){
					ff->RedactFrame(f, -fc);
				} else {
					ff->RemoveFrames(f, -fc);
				}
				changed = true;
			} else if(fc > 0){
				if(e){
					ff->ExtendFrame(f, fc);
				} else {
					ff->AddFrames(f, fc);
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
		this->Broadcast("canvas", MarshalCanvas());
	}

}
void ParupaintServerInstance::ServerFill(int l, int f, QString fill, bool propagate)
{
	QColor col = ParupaintSnippets::toColor(fill);
	canvas->Fill(l, f, col);
	if(record_manager) record_manager->Fill(l, f, fill);

	if(!propagate) return;
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = fill;
	this->Broadcast("fill", obj);
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
		ParupaintLayer * layer = canvas->GetLayer(l);
		if(layer){
			ParupaintFrame * frame = layer->GetFrame(f);
			if(frame){
				frame->DrawImage(x, y, img);
			}
		}
	}
	if(record_manager) record_manager->Paste(l, f, x, y, base64_img);

	if(!propagate) return;
	QJsonObject obj;
	obj["layer"] = l;
	obj["frame"] = f;
	obj["x"] = x;
	obj["y"] = y;
	obj["paste"] = base64_img;
	this->Broadcast("paste", obj);
}

void ParupaintServerInstance::ServerResize(int w, int h, bool r, bool propagate)
{
	if(!r){
		canvas->Clear();
		canvas->AddLayers(0, 1, 1);
	}
	// because resizing only one frame is the quickest.
	canvas->Resize(QSize(w, h));
	if(record_manager) record_manager->Resize(w, h, r);

	if(!propagate) return;
	this->Broadcast("canvas", MarshalCanvas());
}

void ParupaintServerInstance::Message(ParupaintConnection * c, const QString id, const QByteArray bytes)
{
	const QJsonObject obj = QJsonDocument::fromJson(bytes).object();
	QJsonObject obj_copy = obj;
	emit OnMessage(id, obj);

	if(c) {
		if(id == "connect"){

		} else if(id == "join"){
			QString ver = obj["version"].toString("N/A");
			const QString name = obj["name"].toString("unnamed_mofo");
			qDebug() << name << "joined, version" << ver;

			c->setId(connectid++);
			this->ServerJoin(c, name, true);
			this->BroadcastChat(name + " joined.");

			// TODO is there a better way to do this (collect info and send it all?)
			if(record_timer.isActive()){
				QJsonObject obj;
				obj["count"] = record_player->GetTotalLines();
				c->send("play", obj);
			}

		} else if(id == "disconnect") {
			const QString name = this->brushes[c]->GetName();
			this->ServerLeave(c, true);
			this->BroadcastChat(name + " left.");

		} else if(id == "lf") {
			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;
			//TODO rename to lc/fc?
			if(!obj["ll"].isDouble()) return;
			if(!obj["ff"].isDouble()) return;
			this->ServerLfc(obj["l"].toInt(), obj["f"].toInt(),
					obj["ll"].toInt(), obj["ff"].toInt(),
					obj["ext"].toBool(false));

		} else if(id == "fill"){
			if(!obj["l"].isDouble()) return;
			if(!obj["f"].isDouble()) return;
			if(!obj["c"].isString()) return;
			int l = obj["l"].toInt(),
			    f = obj["f"].toInt();
			QString c = obj["c"].toString();

			this->ServerFill(l, f, c);

		} else if(id == "draw"){
			auto * brush = brushes.value(c);
			if(brush) {
				const double 	old_x = brush->GetPosition().x(),
						old_y = brush->GetPosition().y();
				double x = old_x, y = old_y;

				// should come before false->true drawing to move brush
				// to correct pos
				if(obj["x"].isDouble()) x = obj["x"].toDouble();
				if(obj["y"].isDouble()) y = obj["y"].toDouble();

				if(obj["c"].isString()) {
					brush->SetColor(ParupaintSnippets::toColor(obj["c"].toString()));
					if(record_manager) record_manager->Color(c->getId(), obj["c"].toString());
				}
				if(obj["w"].isDouble()) {
					brush->SetWidth(obj["w"].toDouble());
					if(record_manager) record_manager->Width(c->getId(), brush->GetWidth());
				}
				if(obj["t"].isDouble()) {
					brush->SetToolType(obj["t"].toInt());
					if(record_manager) record_manager->Tool(c->getId(), brush->GetToolType());
				}
				if(obj["p"].isDouble()) brush->SetPressure(obj["p"].toDouble());
				if(obj["d"].isBool())	{
					const bool drawing = obj["d"].toBool();
					if(drawing && !brush->IsDrawing()){
						if(record_manager) record_manager->Pos(c->getId(), x, y, brush->GetPressure(), false);
						if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill){
							// ink click
							if(record_manager) record_manager->Pos(c->getId(), x, y, 1, true);
						}
					}
					brush->SetDrawing(drawing);
				}

				if(obj["l"].isDouble()) brush->SetLayer(obj["l"].toInt());
				if(obj["f"].isDouble()) brush->SetFrame(obj["f"].toInt());
				if(obj["l"].isDouble() || obj["f"].isDouble()) {
					if(record_manager) record_manager->Lf(c->getId(), brush->GetLayer(), brush->GetFrame());
				}

				if(obj["x"].isDouble() && obj["y"].isDouble() && brush->IsDrawing()) {
					if(record_manager) record_manager->Pos(c->getId(), x, y, brush->GetPressure(), true);
				}

				if(brush->IsDrawing()){
					ParupaintFrameBrushOps::stroke(canvas, old_x, old_y, x, y, brush);
					if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill){
						brush->SetDrawing(false);
					}
				}

				brush->SetPosition(QPointF(x, y));

				obj_copy["id"] = c->getId();
				this->Broadcast(id, obj_copy);
			}
		} else if(id == "canvas") {
			c->send("canvas", this->MarshalCanvas());
			qDebug("Request canvas");

		} else if (id == "img") {

			for(auto l = 0; l < canvas->GetNumLayers(); l++){
				auto * layer = canvas->GetLayer(l);
				for(auto f = 0; f < layer->GetNumFrames(); f++){
					auto * frame = layer->GetFrame(f);
					if(!layer->IsFrameReal(f)) continue;

					const auto img = frame->GetImage();
					
 					const QByteArray fdata((const char*)img.bits(), img.byteCount());
					QByteArray compressed_bytes;
					QCompressor::gzipCompress(fdata, compressed_bytes);

					QJsonObject obj;
					obj["data"] = QString(compressed_bytes.toBase64());
					obj["w"] = img.width();
					obj["h"] = img.height();
					obj["l"] = l;
					obj["f"] = f;
					c->send("img", QJsonDocument(obj).toJson(QJsonDocument::Compact));
				}
			}
		} else if(id == "save") {
			QSettings cfg;
			auto name = obj["filename"].toString();
			auto load_dir = cfg.value("canvas/directory").toString();

			if(name.section(".", 0, 0).isEmpty()){
				// just put it as today's date then.
				QDateTime time = QDateTime::currentDateTime();
				name = "drawing_at_"+ time.toString("yyyy-MM-dd_HH.mm.ss") + name;
			}

			if(!name.isEmpty()){
				ParupaintPanvasWriter writer(canvas);
				// Loader handles name verification
				auto ret = writer.Save(load_dir, name);
				if(ret == PANVAS_WRITER_RESULT_OK){
					QString msg = QString("Server saved canvas successfully at: \"%1\"").arg(name);
					this->BroadcastChat(msg);
				}
				
			}

		} else if(id == "load") {

			QSettings cfg;
			QString load_dir = cfg.value("server/directory").toString();

			if(obj["file"].isString() && obj["filename"].isString()){

				// a gzipped base64 encoded file is sent, uncompress it and write to temp file
				QByteArray data = QByteArray::fromBase64(obj["file"].toString().toUtf8());
				QByteArray uncompressed;
				QCompressor::gzipDecompress(data, uncompressed);

				QString temp_file = obj["filename"].toString();

				load_dir = QDir::temp().path();
				QFileInfo ff(load_dir, temp_file);

				QFile file(ff.filePath());
				if(!file.open(QIODevice::WriteOnly)){
					obj["filename"] = QJsonValue::Undefined;
					return;
				}
				file.write(uncompressed);
				file.close();
			}

			if(obj["filename"].isString()){
				auto name = obj["filename"].toString();

				if(name.isEmpty()) return;
				ParupaintPanvasReader reader(canvas);
				// Loader handles name verification
				// TODO name verification here pls
				auto ret = reader.Load(load_dir, name);
				if(ret == PANVAS_READER_RESULT_OK){
					qDebug() << "Loaded canvas fine.";
					QString msg = QString("Server loaded file successfully at: \"%1\"").arg(name);
					this->BroadcastChat(msg);

					this->Broadcast("canvas", MarshalCanvas());
				}
			}

		} else if(id == "new") {
			if(!obj["width"].isDouble()) return;
			if(!obj["height"].isDouble()) return;

			int width = obj["width"].toInt();
			int height = obj["height"].toInt();
			bool resize = obj["resize"].toBool();

			if(width <= 0 || height <= 0) return;

			this->ServerResize(width, height, resize);

			QString msg = QString("canvas: %1 x %2").arg(width).arg(height);
			msg.prepend(resize ? "Resize " : "New ");
			this->BroadcastChat(msg);

		} else if(id == "paste"){
			if(!obj["image"].isString()) return;
			QString image = obj["image"].toString();

			if(obj["layer"].isDouble() && obj["frame"].isDouble()){
				int l = obj["layer"].toInt(),
				    f = obj["frame"].toInt(),
				    x = obj["x"].toInt(0),
				    y = obj["y"].toInt(0);
				this->ServerPaste(l, f, x, y, image);
			}

		} else if(id == "chat") {
			if(!obj["message"].isString()) return;
			this->ServerChat(c, obj["message"].toString());

		} else if(id == "play") {
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
				this->Broadcast("play", obj);

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
					this->Broadcast("play", obj);

				} else {
					this->StartRecordTimer();
				}
			}
		} else {
			//qDebug() << id << obj;
		}
	}

}

