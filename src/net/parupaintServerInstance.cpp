#include "parupaintServerInstance.h"

#include <QUrl>
#include <QFileInfo>
#include <QSettings>
#include <QMimeDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QApplication>

#include "parupaintConnection.h"
#include "../core/parupaintRecordManager.h"
#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintBrush.h"
#include "../core/parupaintSnippets.h"

ParupaintServerInstance::~ParupaintServerInstance()
{
	// reset
	record_manager.remove();
}

ParupaintServerInstance::ParupaintServerInstance(quint16 port, QObject * parent) :
	ParupaintServer(port, parent),

	canvas(nullptr), connectid(1),
	ppweb_serve("parupaint-web")
{
	canvas = new ParupaintPanvas(this, QSize(540, 540), 1, 1);

	connect(this, &ParupaintServer::onBrowserVisit, this, &ParupaintServerInstance::browserVisit);

	QSettings server_cfg("server.ini", QSettings::IniFormat, this);
	this->setPassword(server_cfg.value("password").toString());
	this->setParupaintWebServeDir(server_cfg.value("ppweb").toString());
	this->setServerDir(server_cfg.value("server_dir", ".").toString());
}

void ParupaintServerInstance::startRecord()
{
	if(!server_dir.exists()) return;

	QStringList list;
	QFileInfo log(server_dir, ".parupaint.log");

	if(log.exists()){
		// append it - aka don't reset the file

		record_manager.setLogFile(log.filePath(), true);
		qDebug("Recovering log... ");
		while(record_manager.logLines(list));
		qDebug() << "done";
	}

	// NOW reset it.
	record_manager.setLogFile(log.filePath());
	// set initial values
	qDebug() << "Log file:" << log.filePath();
	record_manager.writeLogFile("new", {
		{"w", canvas->dimensions().width()},
		{"h", canvas->dimensions().height()},
		{"r", false}
	});

	if(!list.isEmpty()){
		// and play back the recovery log if there was one.
		this->backupState();
			this->doLines(list);
		this->restoreState();
	}
}

void ParupaintServerInstance::backupState()
{
	record_backup = brushes;
	brushes.clear();

	foreach(ParupaintConnection * con, record_backup.keys()){
		this->leaveConnection(con);
	}
}

void ParupaintServerInstance::restoreState()
{
	while(this->brushes.size()){
		this->leaveConnection(this->brushes.keys().first());
	}

	brushes = record_backup;
	foreach(ParupaintConnection * con, brushes.keys()){
		this->joinConnection(con);
	}

	this->sendInfo();
}

void ParupaintServerInstance::playLogFile(const QString & logfile, int limit)
{
	QFileInfo info(logfile);
	if(!info.exists()) return;

	qDebug() << "Playing log file" << info.filePath();

	ParupaintRecordManager rec;
	rec.setLogFile(logfile, true);
	rec.resetLogReader();

	this->backupState();

	// perform
	QStringList list;
	int i = 0;
	while(rec.logLines(list)){
		this->doLines(list);

		QElapsedTimer timer;
		timer.start();
		while(timer.elapsed() < 200)
			QApplication::processEvents();

		if(limit != -1 && (++i) > limit) break;
	}

	this->restoreState();
}

void ParupaintServerInstance::doLine(const QString & line)
{
	if(line.indexOf('{') == line.indexOf(' ')+1){

		QString name = line.section(' ', 0, 0);
		QJsonObject obj = QJsonDocument::fromJson(line.section(' ', 1).toUtf8()).object();

		if(line.indexOf(':') < line.indexOf(' ')){
			obj["id"] = name.section(':', 1).toInt();
			name = name.section(':', 0, 0);
		}
		if(name == "join"){
			ParupaintConnection * con = new ParupaintConnection(nullptr);
			con->setId(obj["id"].toInt());
			brushes[con] = new ParupaintBrush;

			obj["exists"] = true;
			name = "brush";
		}
		ParupaintConnection * delete_after = nullptr;

		if(name == "leave"){
			foreach(ParupaintConnection * c, this->brushes.keys()){
				if(c->id() == obj["id"].toInt()) { delete_after = c; break; }
			}
			obj["exists"] = false;
			name = "brush";
		}

		this->doMessage(name, obj);

		if(delete_after){
			delete brushes.take(delete_after);
			delete delete_after;
		}
	}
}

void ParupaintServerInstance::doLines(const QStringList & lines)
{
	if(lines.isEmpty()) return;

	for(int i = 0; i < lines.size(); i++){
		this->doLine(lines.at(i));
	}
}

void ParupaintServerInstance::browserVisit(QTcpSocket * socket, const QString & path)
{
	QUrl url(path);

	if(url.path().startsWith("/image")){

		QStringList http_response = {
			"HTTP/1.1 200 OK",
			"Content-Type: image/png",
			"Connection: close",
			"\r\n"
		};
		// if(url.query() == "refresh") http_response.insert(http_response.length()-1, "Refresh: 1");

		socket->write(http_response.join("\r\n").toUtf8());
		canvas->mergedImageFrames(false).first().save(socket, "png");

	} else if(path == "/ping"){

		socket->write("pong");

	} else {

		QString fixed_file = url.path().mid(1);
		if(fixed_file.isEmpty())
			fixed_file = "index.html";

		QFileInfo pp(ppweb_serve, fixed_file);
		if(pp.exists()){
			QFile file(pp.filePath());
			if(file.open(QIODevice::ReadOnly)){
				QMimeDatabase db;
				QString mime = db.mimeTypeForFile(pp).name();

				QStringList http_response = {
					"HTTP/1.1 200 OK",
					("Content-Type: " + mime),
					"Connection: close",
					"\r\n"
				};
				socket->write(http_response.join("\r\n").toUtf8());
				socket->write(file.readAll());
				file.close();
			}
		}
	}
}

void ParupaintServerInstance::joinConnection(ParupaintConnection * con)
{
	if(!con) return;
	if(!brushes[con]){
		brushes[con] = new ParupaintBrush();
		brushes[con]->setName(con->name());
	}

	con->send("join");
	emit onJoin(con);

	// msg
	QJsonObject obj = this->connectionObj(con);
	record_manager.writeLogFile("join", obj);

	obj["exists"] = true;
	this->sendAll("brush", obj, con);
}

void ParupaintServerInstance::leaveConnection(ParupaintConnection * con)
{
	QJsonObject obj = this->connectionObj(con);
	record_manager.writeLogFile("leave", obj);

	obj["exists"] = false;
	this->sendAll("brush", obj, con);

	if(!con) return;
	if(brushes.find(con) == brushes.end()) return;
	delete brushes.take(con);
}

ParupaintConnection * ParupaintServerInstance::getConnection(int id)
{
	ParupaintConnection * con = nullptr;
	foreach(ParupaintConnection * c, this->brushes.keys()){
		if(c->id() != id) continue;
		con = c; break;
	}
	return con;
}

void ParupaintServerInstance::setBrushesDrawing(bool stopdraw)
{
	foreach(ParupaintBrush * brush, this->brushes){
		brush->setDrawing(stopdraw);
	}
}

void ParupaintServerInstance::objMessage(const QString & id, const QJsonObject & obj)
{
	this->sendAll(id, obj);
	record_manager.writeLogFile(id, obj);
}

void ParupaintServerInstance::sendInfo()
{
	QJsonObject obj;
	obj["painters"] = this->numPainters();
	obj["spectators"] = this->numSpectators();
	obj["password"] = !this->password().isEmpty();

	this->sendAll("info", obj);
}

void ParupaintServerInstance::setParupaintWebServeDir(QDir dir)
{
	if(!dir.exists()) return;
	QFileInfo index(dir, "index.html");
	if(!index.exists()) return;

	this->ppweb_serve = dir;
}
const QDir & ParupaintServerInstance::parupaintWebServeDir() const
{
	return ppweb_serve;
}

void ParupaintServerInstance::setServerDir(QDir dir)
{
	if(!dir.exists()) return;
	this->server_dir = dir;
}
const QDir & ParupaintServerInstance::serverDir() const
{
	return server_dir;
}

void ParupaintServerInstance::setPassword(const QString & password)
{
	this->server_password = password;
}

const QString & ParupaintServerInstance::password()
{
	return this->server_password;
}

ParupaintPanvas * ParupaintServerInstance::GetCanvas()
{
	return canvas;
}

QJsonObject ParupaintServerInstance::connectionObj(ParupaintConnection * con) const
{
	QJsonObject obj = {
		{"id", con->id()},
		{"n", con->name()}
	};

	ParupaintBrush * brush = this->brushes.value(con);
	if(brush){
		obj["x"] = brush->x();
		obj["y"] = brush->y();
		obj["s"] = brush->size();
		obj["t"] = brush->tool();
		obj["l"] = brush->layer();
		obj["f"] = brush->frame();
	}

	return obj;
}

QJsonObject ParupaintServerInstance::canvasObj() const
{
	QJsonObject obj = canvas->json();
	return obj;
}

void ParupaintServerInstance::sendAll(const QString & id, const QJsonObject & obj, ParupaintConnection * con)
{
	foreach(ParupaintConnection * c, this->ppConnections()){
		if(c == con) continue;
		c->send(id, obj);
	}
}
void ParupaintServerInstance::sendChat(const QString & str, ParupaintConnection * con)
{
	QJsonObject obj;
	obj["message"] = str;
	this->sendAll("chat", obj, con);
}

int ParupaintServerInstance::numSpectators() const
{
	return (this->numConnections() - this->numPainters());
}

int ParupaintServerInstance::numPainters() const
{
	return brushes.size();
}

int ParupaintServerInstance::numConnections() const
{
	return this->ppNumConnections();
}
