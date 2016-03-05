#ifndef PARUPAINTRECORDFILE_H
#define PARUPAINTRECORDFILE_H

#include <QFile>
#include <QElapsedTimer>

class ParupaintRecordFile : public QFile
{
	QElapsedTimer timer;

	public:
	ParupaintRecordFile();

	bool openLog(const QString & file);
	void writeLog(const QString & name, const QString & data = QString());
	void resetLog();
};

#endif
