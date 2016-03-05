#ifndef PARUPAINTRECORDMANAGER_H
#define PARUPAINTRECORDMANAGER_H

#include "parupaintRecordFile.h"
#include <QTextStream>
#include <QJsonObject>

class ParupaintRecordManager
{
	private:
	ParupaintRecordFile log;
	QTextStream text_stream;

	public:
	ParupaintRecordManager();
	~ParupaintRecordManager();

	void setLogFile(const QString & file, bool append = false);

	void writeLogFile(QString name, QJsonObject data);
	void writeLogFile(const QString & name, const QString & data = QString());

	bool logLine(QString * str = nullptr);
	bool logLines(QStringList & list);
	void resetLogReader();
	void remove();

	QTextStream & textStream();
};

#endif
