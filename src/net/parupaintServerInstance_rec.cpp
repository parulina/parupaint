#include "parupaintServerInstance.h"

#include "parupaintConnection.h"
#include "../core/parupaintRecordManager.h"
#include "../core/parupaintRecordPlayer.h"

#include "../core/parupaintPanvas.h"
#include "../core/parupaintBrush.h"
#include "../core/parupaintFrameBrushOps.h"
#include "../core/parupaintSnippets.h"

#include <QJsonObject>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include <QDebug>

QString log_path = "./.parupaint.log";


void ParupaintServerInstance::StopRecordSystems()
{
	if(record_player) delete record_player;
	if(record_manager) delete record_manager;

	QFile::remove(log_path);
}

void ParupaintServerInstance::StartRecordSystems()
{
	record_timer.stop();
	connect(&record_timer, &QTimer::timeout, this, &ParupaintServerInstance::RecordTimerStep, Qt::UniqueConnection);

	// Get log path
	QSettings cfg;
	log_path = QDir(QCoreApplication::applicationDirPath()).filePath(".parupaint.log");
	log_path = cfg.value("client/logfile", log_path).toString();
	const QString log_bak = log_path + ".bak";

	qDebug() << "Log file:" << log_path;

	if(QFile(log_path).exists()){
		QFile::rename(log_path, log_bak);
	}

	// Do actual init
	record_player = new ParupaintRecordPlayer;
	record_manager = new ParupaintRecordManager(log_path);

	if(canvas){
		record_manager->Resize(canvas->dimensions().width(), canvas->dimensions().height(), false);
	}

	if(cfg.value("client/recoverlogfile", true).toBool()){
		qDebug() << "Playing old recovery file" << log_bak;
		this->PlayRecordLog(log_bak);
		QFile::remove(log_bak);
	}
}

// WARNING this will erase all painters!
// Use only for log recovery
void ParupaintServerInstance::PlayRecordLog(const QString & file)
{
	QFile log_file(file);
	if(log_file.open(QFile::ReadOnly | QIODevice::Text)){

		// go through log file
		QTextStream recovery_stream(log_file.readAll());
		while(!recovery_stream.atEnd()){
			const QString line = recovery_stream.readLine();
			this->RecordLineDecoder(line, true);
		}
		// clear out brushes
		foreach(auto con, brushes.keys()){
			this->ServerLeave(con, false);
			delete con;
		}
		brushes.clear();
	}
}

void ParupaintServerInstance::RecordLineDecoder(const QString & line, bool recovery)
{
	if(line.isEmpty()) return;
	QString line_temp = line.trimmed();
	if(line_temp[0] == ';') return;

	if(line_temp.contains(" ;")) line_temp = line.section(" ;", 0, 0).trimmed();
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

	} else if(id == "paste" && ct == 5){
		int 	l = str.takeFirst().toInt(),
			f = str.takeFirst().toInt(),
			x = str.takeFirst().toInt(),
			y = str.takeFirst().toInt();
		QString base64 = str.takeFirst();
		this->ServerPaste(l, f, x, y, base64);
		return;

	} else if(id == "keep") {
		if(record_player) record_player->SetWillRestore(false);
		return;

	} else if(id == "restore") {
		if(record_player) record_player->SetWillRestore(true);
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
		if(k->id() == b) {con = k; break;}
	}

	// con should now hold a ParupaintConnection
	// if they exist, otherwise ignore it
	if(id == "join") {
		if(!con) {
			// create a connection without any socket -- dummy connection
			con = new ParupaintConnection(nullptr);
			con->setId(b);
		}
		this->ServerJoin(con, !recovery);
		if(str.length() == 1){
			this->ServerName(con, str.takeFirst(), !recovery);
		}
		return;
	}
	if(con){
		if(id == "leave") {
			this->ServerLeave(con, !recovery);

		} else if(id == "name") {
			this->ServerName(con, str.takeFirst(), !recovery);

		// these are written at "draw" usually.
		} else if(id == "width" && ct == 1) {
			qreal size = str.takeFirst().toDouble();
			brushes[con]->setSize(size);
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["s"] = size;
				this->sendAll("draw", obj);
			} else {
				if(record_manager) record_manager->Width(b, size);
			}

		} else if(id == "chat") {
			// skip out id + b
			QString msg = line_temp.section(' ', 2);
			if(!msg.isEmpty()) this->ServerChat(con, msg, !recovery);

		} else if(id == "color" && ct == 1) {
			QString col = str.takeFirst();
			brushes[con]->setColor(ParupaintSnippets::toColor(col));
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["c"] = col;
				this->sendAll("draw", obj);
			} else {
				if(record_manager) record_manager->Color(b, col);
			}

		} else if(id == "tool" && ct == 1) {
			int tool = str.takeFirst().toInt();
			brushes[con]->setTool(tool);
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["t"] = tool;
				this->sendAll("draw", obj);
			} else {
				if(record_manager) record_manager->Tool(b, tool);
			}

		} else if(id == "lf" && ct == 2) {
			const int 	l = str.takeFirst().toInt(),
					f = str.takeFirst().toInt();
			brushes[con]->setLayerFrame(l, f);
			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["l"] = l;
				obj["f"] = f;
				this->sendAll("draw", obj);
			} else {
				if(record_manager) record_manager->Lf(b, l, f);
			}

		} else if(id == "pos" && ct == 4) {
			const double 	old_x = brushes[con]->x(),
					old_y = brushes[con]->y();

			const double 	x = str.takeFirst().toDouble(),
			      		y = str.takeFirst().toDouble(),
					p = str.takeFirst().toDouble();
			const bool	d = str.takeFirst().toInt();

			brushes[con]->setPosition(QPointF(x, y));
			brushes[con]->setPressure(p);
			brushes[con]->setDrawing(d);

			if(brushes[con]->drawing()){
				ParupaintFrameBrushOps::stroke(canvas, brushes[con], QPointF(x, y), QPointF(old_x, old_y));
			}

			if(!recovery){
				QJsonObject obj;
				obj["id"] = b;
				obj["x"] = x;
				obj["y"] = y;
				obj["p"] = p;
				obj["d"] = d;
				this->sendAll("draw", obj);
			} else {
				if(record_manager) record_manager->Pos(b, x, y, p, d);
			}
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

		b->copyTo(*record_backup[c]);
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
			backup_it.value()->copyTo(*b);
			record_backup.erase(backup_it);
		}
		QJsonObject obj;
		obj["s"] = b->size();
		obj["t"] = b->tool();
		obj["c"] = b->colorString();

		obj["id"] = c->id();
		obj["d"] = false;
		obj["l"] = b->layer();
		obj["f"] = b->frame();
		this->sendAll("draw", obj);
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
			this->sendAll("play", obj);

			record_player->Reset();
			record_timer.stop();
		}
	}
}
