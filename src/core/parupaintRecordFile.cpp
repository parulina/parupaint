#include "parupaintRecordFile.h"

#include <QStringBuilder>
#include <QTime>
#include <QtMath>
#include <QDebug>

ParupaintRecordFile::ParupaintRecordFile()
{
	timer.start();
}

bool ParupaintRecordFile::openLog(const QString & file)
{
	if(this->isOpen()) this->close();

	this->setFileName(file);
	return this->open(QIODevice::ReadWrite);
}

void ParupaintRecordFile::writeLog(const QString & name, const QString & data)
{
	if(!this->isOpen() || !this->isWritable()) return;

	// write tick if applicable
	qint64 elapsed = timer.restart();
	if(elapsed > 100) {
		// 100 msec has passed, write a "tick" msg
		this->write(QString("tick %1\n").arg(qFloor(elapsed / 100.0) * 100.0).toUtf8());
	}

	// write the data
	this->write(name.toUtf8());
	if(!data.isEmpty()){
		this->write(" ");
		this->write(data.toUtf8());
	}

	this->write("\n");

	this->flush();
}

// remember to reset the log if you want to start fresh.
void ParupaintRecordFile::resetLog()
{
	this->resize(0);
}
