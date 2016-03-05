#include "parupaintRecordManager.h"

#include <QLocale>
#include <QTextStream>
#include <QPointF>
#include <QColor>
#include <QSize>

#include <QJsonDocument>
#include <QtMath>
#include <QDebug>

#define PARUPAINT_ENDL qSetFieldWidth(0) << endl << qSetFieldWidth(3)

// manages the record file, storing snapshots and current line
// also provides batches of lines for each tick

ParupaintRecordManager::~ParupaintRecordManager()
{
}

ParupaintRecordManager::ParupaintRecordManager() :
	text_stream(&log)
{
}

void ParupaintRecordManager::setLogFile(const QString & file, bool append)
{
	log.openLog(file);
	if(!append) {
		log.resetLog();
	}

	text_stream.reset();
}

void ParupaintRecordManager::writeLogFile(QString name, QJsonObject data)
{
	if(data.isEmpty()) this->writeLogFile(name);

	if(data.contains("id") && data["id"].isDouble())
		name = QString("%1:%2").arg(name).arg(data.take("id").toInt(0));

	if(data.contains("x") && data.value("x").isDouble())
		data["x"] = qFloor(data["x"].toDouble());

	if(data.contains("y") && data.value("y").isDouble())
		data["y"] = qFloor(data["y"].toInt());

	this->writeLogFile(name, QJsonDocument(data).toJson(QJsonDocument::Compact));
}

void ParupaintRecordManager::writeLogFile(const QString & name, const QString & data)
{
	log.writeLog(name, data);
}

bool ParupaintRecordManager::logLine(QString * str)
{
	bool r;
	do {
		// anything over a MB - can't do
		r = text_stream.readLineInto(str, 1024*1024);

	} while((str->trimmed().startsWith('#') || str->isEmpty()) && r);
	
	return r;
}

bool ParupaintRecordManager::logLines(QStringList & list)
{
	if(text_stream.atEnd()) return false;

	list.clear();

	QString line;
	while(this->logLine(&line)){
		if(line.startsWith("tick")) break;
		list.append(line);
	}
	return true;
}

void ParupaintRecordManager::resetLogReader()
{
	text_stream.seek(0);
}

void ParupaintRecordManager::remove()
{
	log.remove();
}

QTextStream & ParupaintRecordManager::textStream()
{
	return text_stream;
}
