
#include <QLocale>
#include <QTextStream>
#include <QPointF>
#include <QColor>
#include <QSize>

#include <QDebug>

#include "parupaintRecordManager.h"
#define PARUPAINT_ENDL qSetFieldWidth(0) << endl << qSetFieldWidth(3)

ParupaintRecordManager::~ParupaintRecordManager()
{
	temp_stream->flush();
	temp_log.close();
	delete temp_stream;
}
ParupaintRecordManager::ParupaintRecordManager(QString log) : temp_log(log), temp_stream(nullptr)
{
	if(temp_log.open(QFile::WriteOnly)){
		qDebug() << "Session log file open.";
		temp_stream = new QTextStream(&temp_log);
	}
}

void ParupaintRecordManager::Write(QStringList list)
{
	*temp_stream << list.join(' ') << endl;
	temp_stream->flush();
}

void ParupaintRecordManager::Join(int id, QString name)
{
	if(temp_stream){
		this->Write({"join", QString::number(id), name});
	}
}
void ParupaintRecordManager::Leave(int id)
{
	if(temp_stream){
		this->Write({"leave", QString::number(id)});
	}
}
void ParupaintRecordManager::Pos(int id, int x, int y, double p, bool d)
{
	if(temp_stream){
		this->Write({"pos", QString::number(id), QString::number(x), QString::number(y), QString::number(p), QString::number(d)});
	}
}
void ParupaintRecordManager::Tool(int id, int t)
{
	if(temp_stream){
		this->Write({"tool", QString::number(id), QString::number(t)});
	}
}
void ParupaintRecordManager::Lf(int id, int l, int f)
{
	if(temp_stream){
		this->Write({"lf", QString::number(id), QString::number(l), QString::number(f)});
	}
}
void ParupaintRecordManager::Color(int id, QString col)
{
	if(temp_stream){
		this->Write({"color", QString::number(id), col});
	}
}
void ParupaintRecordManager::Width(int id, double w)
{
	if(temp_stream){
		this->Write({"width", QString::number(id), QString::number(w)});
	}
}
void ParupaintRecordManager::Chat(int id, QString chat)
{
	if(temp_stream){
		this->Write({"chat", QString::number(id), chat});
	}
}
void ParupaintRecordManager::Fill(int l, int f, QString col)
{
	if(temp_stream){
		this->Write({"fill", QString::number(l), QString::number(f), col});
	}
}
void ParupaintRecordManager::Resize(int width, int height, bool clear)
{
	if(temp_stream){
		this->Write({"resize", QString::number(width), QString::number(height), QString::number(clear)});
	}
}
void ParupaintRecordManager::Lfc(int l, int f, int lc, int fc, bool ext)
{
	if(temp_stream){
		this->Write({"lfc", QString::number(l), QString::number(f), QString::number(lc), QString::number(fc), QString::number(ext)});
	}
}

void ParupaintRecordManager::Reset()
{
	if(temp_stream){
		temp_log.resize(0);
		temp_stream->seek(0);
	}
}
