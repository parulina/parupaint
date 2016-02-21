#include "parupaintServerInstance.h"

#include <QUrl>
#include <QFileInfo>
#include <QSettings>
#include <QMimeDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "parupaintConnection.h"
#include "../core/parupaintPanvas.h"
#include "../core/parupaintLayer.h"
#include "../core/parupaintFrame.h"
#include "../core/parupaintBrush.h"
#include "../core/parupaintSnippets.h"

ParupaintServerInstance::~ParupaintServerInstance()
{
	this->StopRecordSystems();
}

ParupaintServerInstance::ParupaintServerInstance(quint16 port, QObject * parent) : ParupaintServer(port, parent),
	canvas(nullptr), connectid(1),
	ppweb_serve("parupaint-web"),
	record_player(nullptr), record_manager(nullptr)
{
	connect(this, &ParupaintServer::onBrowserVisit, this, &ParupaintServerInstance::browserVisit);

	QSettings server_cfg("server.ini", QSettings::IniFormat, this);
	this->setPassword(server_cfg.value("password").toString());
	this->setParupaintWebServeDir(server_cfg.value("ppweb").toString());

	canvas = new ParupaintPanvas(this, QSize(540, 540), 1, 1);

	this->StartRecordSystems();
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
	con->send("join");
	emit onJoin(con);

	this->ServerJoin(con, true);
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
