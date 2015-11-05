
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#include <QTimer>
#include <QFile>
#include <QTextStream>

#include "parupaintConnection.h"
#include "parupaintServerInstance.h"
#include "../core/parupaintPanvasReader.h"
#include "../core/parupaintPanvasWriter.h"
#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintSnippets.h"
#include "../core/parupaintRecordManager.h"
#include "../core/parupaintRecordPlayer.h"

#include "../core/parupaintFrameBrushOps.h"
#include "../core/parupaintBrush.h"

#include "qcompressor.h"

QString log_path = "./.parupaint.log";

ParupaintServerInstance::~ParupaintServerInstance()
{
	delete record_player;
	delete record_manager;

	QFile::remove(log_path);
}

ParupaintServerInstance::ParupaintServerInstance(quint16 port, QObject * parent) : ParupaintServer(port, parent)
{
	connectid = 1;
	canvas = new ParupaintPanvas(540, 540, 1);

	log_path = QDir(QCoreApplication::applicationDirPath()).filePath(".parupaint.log");
	qDebug() << "Log" << log_path;

	QFile log_file(log_path);
	if(log_file.open(QFile::ReadOnly)){
		log_recovery = log_file.readAll();
		log_file.close();
	}
	record_player = new ParupaintRecordPlayer();
	record_manager = new ParupaintRecordManager(log_path);
	record_manager->Resize(540, 540, false);
	// this adds it to history
	this->ServerFill(0, 0, "#FFF", false);

	if(!log_recovery.isEmpty()){
		QTextStream recovery_stream(log_recovery);
		while(!recovery_stream.atEnd()){
			const QString line = recovery_stream.readLine();
			this->RecordLineDecoder(line, true);
		}
	}
	foreach(auto con, brushes.keys()){
		// TODO idk if i need to delete the keys...
		record_manager->Leave(con->getId());
		delete brushes[con];
	}
	brushes.clear();

	connect(this, &ParupaintServer::onMessage, this, &ParupaintServerInstance::Message);

	connect(&record_timer, &QTimer::timeout, this, &ParupaintServerInstance::RecordTimerStep);
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QString ParupaintServerInstance::MarshalCanvas()
{
	QJsonObject obj;
	obj["width"] = canvas->GetWidth();
	obj["height"] = canvas->GetHeight();

	QJsonArray layers;
	for(auto l = 0; l < canvas->GetNumLayers(); l++){
		auto * layer = canvas->GetLayer(l);
		if(!layer) continue;

		QJsonArray frames;
		for(auto f = 0; f < layer->GetNumFrames(); f++){
			auto * frame = layer->GetFrame(f);

			QJsonObject fobj;
			fobj["extended"] = !(layer->IsFrameReal(f));
			fobj["opacity"] = frame->GetOpacity();

			frames.insert(f, fobj);
		}
		layers.insert(l, frames);
	}
	obj["layers"] = layers;
		

	return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QJsonObject ParupaintServerInstance::MarshalConnection(ParupaintConnection* connection)
{
	QJsonObject obj;
	obj["id"] = connection->getId();

	obj["name"] = brushes[connection]->GetName();
	obj["x"] = brushes[connection]->GetPosition().x();
	obj["y"] = brushes[connection]->GetPosition().y();
	obj["w"] = brushes[connection]->GetWidth();
	
	return obj;
}

int ParupaintServerInstance::GetNumConnections()
{
	return brushes.size();
}


// recovery - do a fast forward 'merge' of this history with current history,
// skips broadcasts and forces readd of entries
void ParupaintServerInstance::RecordLineDecoder(const QString & line, bool recovery)
{
	if(line.isEmpty()) return;
	if(line.trimmed()[0] == ';') return;
	QString line_temp = line.trimmed();

	if(line_temp.contains(';')) line_temp = line.section(';', 0, 0).trimmed();
	QStringList str = line_temp.split(' ');

	const QString id = str.takeFirst();
	int ct = str.length();

	// these come first as they have no player id attached to them.
	// return to skip the rest of the func
	if(id == "resize" && ct == 3){
		int w = str.takeFirst().toInt();
		int h = str.takeFirst().toInt();
		bool r = str.takeFirst().toInt();
		this->ServerResize(w, h, r, !recovery);
		return;

	} else if(id == "lfc" && ct == 5) {
		int 	l = str.takeFirst().toInt(),
			f = str.takeFirst().toInt(),
			lc = str.takeFirst().toInt(),
			fc = str.takeFirst().toInt();
		bool 	e = str.takeFirst().toInt();

		this->ServerLfc(l, f, lc, fc, e, !recovery);
		return;

	} else if(id == "fill" && ct == 3) {
		int 	l = str.takeFirst().toInt(),
			f = str.takeFirst().toInt();
		QString c = str.takeFirst();
		this->ServerFill(l, f, c, !recovery);
		return;

	} else if(id == "keep") {
		record_player->SetWillRestore(false);
		return;

	} else if(id == "restore") {
		record_player->SetWillRestore(true);
		return;
	}

	bool bt = false;
	const int b = str.takeFirst().toInt(&bt);
	if(!bt) return;
	ct = str.length();


	// start
	ParupaintConnection * con = nullptr;
	foreach(auto k, brushes.keys()){
		// find connection from id
		if(k->getId() == b) {con = k; break;}
	}

	if(id == "join") {
		if(!con) {
			con = new ParupaintConnection(nullptr);
			con->setId(b);
		}
		this->ServerJoin(con, str.takeFirst(), !recovery);
		return;
	}
	if(con){
		if(id == "leave") {
			this->ServerLeave(con, !recovery);

		// these are written at "draw" usually.
		} else if(id == "width" && ct == 1) {
			brushes[con]->SetWidth(str.takeFirst().toDouble());
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["w"] = brushes[con]->GetWidth();
				this->Broadcast("draw", obj);
			} else {
				record_manager->Width(b, brushes[con]->GetWidth());
			}

		} else if(id == "chat") {
			// skip out id + b
			QString msg = line.section(' ', 2);
			if(!msg.isEmpty()) this->ServerChat(con, msg, !recovery);

		} else if(id == "color" && ct == 1) {
			QString col = str.takeFirst();
			brushes[con]->SetColor(ParupaintSnippets::toColor(col));
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["c"] = col;
				this->Broadcast("draw", obj);
			} else {
				record_manager->Color(b, col);
			}

		} else if(id == "tool" && ct == 1) {
			brushes[con]->SetToolType(str.takeFirst().toInt());
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["t"] = brushes[con]->GetToolType();
				this->Broadcast("draw", obj);
			} else {
				record_manager->Tool(b, brushes[con]->GetToolType());
			}

		} else if(id == "lf" && ct == 2) {
			const int 	l = str.takeFirst().toInt(),
					f = str.takeFirst().toInt();
			brushes[con]->SetLayer(l);
			brushes[con]->SetFrame(f);
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["l"] = l;
				obj["f"] = f;
				this->Broadcast("draw", obj);
			} else {
				record_manager->Lf(b, l, f);
			}

		} else if(id == "pos" && ct == 4) {
			const double 	old_x = brushes[con]->GetPosition().x(),
					old_y = brushes[con]->GetPosition().y();

			const double 	x = str.takeFirst().toDouble(),
			      		y = str.takeFirst().toDouble(),
					p = str.takeFirst().toDouble();
			const bool	d = str.takeFirst().toInt();

			brushes[con]->SetPosition(QPointF(x, y));
			brushes[con]->SetPressure(p);
			brushes[con]->SetDrawing(d);

			if(brushes[con]->IsDrawing()){
				ParupaintFrameBrushOps::stroke(canvas, old_x, old_y, x, y, brushes[con]);
			}

			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["x"] = x;
				obj["y"] = y;
				obj["p"] = p;
				obj["d"] = d;
				this->Broadcast("draw", obj);
			} else {
				record_manager->Pos(b, x, y, p, d);
			}
		}
	}
}

// Makes someone join the server with a given name.
// a brush is created.
void ParupaintServerInstance::ServerJoin(ParupaintConnection * c, QString name, bool propagate)
{
	if(!c) return;

	if(!brushes[c]){
		brushes[c] = new ParupaintBrush();
	}
	brushes[c]->SetName(name);
	record_manager->Join(c->getId(), name);

	//FIXME fix in parupaint-web so this isn't needed anymore
	//c->send("join", "{\"name\":\"sqnya\"}");

	if(!propagate) return;
	// TODO is there a better way to do this (collect info and send it all?)
	if(record_timer.isActive()){
		QJsonObject obj;
		obj["count"] = record_player->GetTotalLines();
		c->send("play", QJsonDocument(obj).toJson(QJsonDocument::Compact));
	}
	foreach(auto c2, brushes.keys()){

		QJsonObject obj_me = this->MarshalConnection(c);
		if(c2 == c) obj_me["id"] = -(c->getId());
		obj_me["disconnect"] = false;

		// send me to others
		c2->send("peer", QJsonDocument(obj_me).toJson(QJsonDocument::Compact));

		if(c2 == c) continue;

		// send them to me
		QJsonObject obj_them = this->MarshalConnection(c2);
		c->send("peer", QJsonDocument(obj_them).toJson(QJsonDocument::Compact));
	}
}
void ParupaintServerInstance::ServerLeave(ParupaintConnection * c, bool propagate)
{
	if(!c) return;
	if(brushes.find(c) == brushes.end()) return;
	delete brushes[c];
	brushes.remove(c);

	record_manager->Leave(c->getId());

	if(!propagate) return;
	QJsonObject obj;
	obj["disconnect"] = true;
	obj["id"] = c->getId();
	this->Broadcast("peer", obj);
}
void ParupaintServerInstance::ServerChat(ParupaintConnection * c, QString msg, bool propagate)
{
	record_manager->Chat(c->getId(), msg);

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

	record_manager->Lfc(l, f, lc, fc, e);

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
	record_manager->Fill(l, f, fill);

	if(!propagate) return;
	QJsonObject obj;
	obj["l"] = l;
	obj["f"] = f;
	obj["c"] = fill;
	this->Broadcast("fill", obj);
}

void ParupaintServerInstance::ServerPaste(int l, int f, int x, int y, QString base64_img, bool propagate)
{
	// paste has to be a base64 thing.
	QImage img = ParupaintSnippets::toImage(base64_img);
	if(!img.isNull()){
		ParupaintLayer * layer = canvas->GetLayer(l);
		if(layer){
			ParupaintFrame * frame = layer->GetFrame(f);
			if(frame){
				frame->DrawImage(x, y, img);
			}
		}
	}
	this->record_manager->Paste(l, f, x, y, base64_img);

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
	record_manager->Resize(w, h, r);

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
					record_manager->Color(c->getId(), obj["c"].toString());
				}
				if(obj["w"].isDouble()) {
					brush->SetWidth(obj["w"].toDouble());
					record_manager->Width(c->getId(), brush->GetWidth());
				}
				if(obj["t"].isDouble()) {
					brush->SetToolType(obj["t"].toInt());
					record_manager->Tool(c->getId(), brush->GetToolType());
				}
				if(obj["p"].isDouble()) brush->SetPressure(obj["p"].toDouble());
				if(obj["d"].isBool())	{
					const bool drawing = obj["d"].toBool();
					if(drawing && !brush->IsDrawing()){
						record_manager->Pos(c->getId(), x, y, brush->GetPressure(), false);
						if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill){
							// ink click
							record_manager->Pos(c->getId(), x, y, 1, true);
						}
					}
					brush->SetDrawing(drawing);
				}

				if(obj["l"].isDouble()) brush->SetLayer(obj["l"].toInt());
				if(obj["f"].isDouble()) brush->SetFrame(obj["f"].toInt());
				if(obj["l"].isDouble() || obj["f"].isDouble()) {
					record_manager->Lf(c->getId(), brush->GetLayer(), brush->GetFrame());
				}

				if(obj["x"].isDouble() && obj["y"].isDouble() && brush->IsDrawing()) {
					record_manager->Pos(c->getId(), x, y, brush->GetPressure(), true);
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

		} else if(id == "load") {

			QSettings cfg;
			QString load_dir = cfg.value("server/directory").toString();

			if(obj["file"].isString() && obj["filename"].isString()){

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
void ParupaintServerInstance::StartRecordTimer()
{
	record_timer.stop();
	record_timer.start(2);
}

void ParupaintServerInstance::SaveRecordBrushes()
{
	record_backup.clear();
	for(auto i = brushes.begin(); i != brushes.end(); ++i){
		ParupaintConnection * c = i.key();
		ParupaintBrush * b = i.value();

		record_backup[c] = *b;
	}
}
void ParupaintServerInstance::RestoreRecordBrushes()
{
	if(record_backup.isEmpty()) return;
	qDebug() << "Restoring brushes.";
	for(auto i = brushes.begin(); i != brushes.end(); ++i){
		ParupaintConnection * c = i.key();
		ParupaintBrush * b = i.value();

		// trawl through saved brushes and restore em
		auto backup_it = record_backup.find(c);
		if(backup_it != record_backup.end()){
			*b = backup_it.value();
			record_backup.erase(backup_it);
		}
		QJsonObject obj;

		//     /\
		//    /  \
		//   /.--.\
		//  / '--' \
		// /________\
		//
		obj["w"] = b->GetWidth();
		obj["t"] = b->GetToolType();
		obj["c"] = b->GetColorString();

		obj["id"] = c->getId();
		obj["d"] = false;
		obj["l"] = b->GetLayer();
		obj["f"] = b->GetFrame();
		this->Broadcast("draw", obj);
	}
	record_backup.clear();
}

void ParupaintServerInstance::RecordTimerStep()
{
	if(record_player->IsSleeping()){
		record_player->SetSleeping(false);
		this->StartRecordTimer();
	}
	if(record_player->GetTotalLines()){
		QString line;
		if(!record_player->TakeLine(line)){
			if(line.startsWith("sleep") || line.startsWith("interval")){
				bool k;
				const double time = line.section(' ', 1, 1).toDouble(&k);
				if(k){
					// if it's sleep, reset it the next timer tick
					if(line.startsWith("sleep")) record_player->SetSleeping(true);
					record_timer.setInterval(time * 1000);
				}
			}
			this->RecordLineDecoder(line, false);
		} else {
			// Reached end
			qDebug() << "End of play";
			if(record_player->WillRestore()){
				this->RestoreRecordBrushes();
			}

			QJsonObject obj;
			obj["count"] = 0;
			this->Broadcast("play", obj);

			record_player->Reset();
			record_timer.stop();
		}
	}
}

void ParupaintServerInstance::BroadcastChat(QString str)
{
	QJsonObject obj;
	obj["message"] = str;
	this->Broadcast("chat", obj);
}

void ParupaintServerInstance::Broadcast(QString id, QJsonObject ba, ParupaintConnection * c)
{
	this->Broadcast(id, QJsonDocument(ba).toJson(QJsonDocument::Compact), c);
}
void ParupaintServerInstance::Broadcast(QString id, QString ba, ParupaintConnection * c)
{
	return this->Broadcast(id, ba.toUtf8(), c);
}
void ParupaintServerInstance::Broadcast(QString id, const QByteArray ba, ParupaintConnection * c)
{
	foreach(auto i, brushes.keys()){
		if(i != c){
			if(i->getSocket()) i->send(id, ba);
		}
	}
}

