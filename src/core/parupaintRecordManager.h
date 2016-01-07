#ifndef PARUPAINTRECORDMANAGER_H
#define PARUPAINTRECORDMANAGER_H

#include <QFile>

class QTextStream;

class ParupaintRecordManager
{
	private:
	QFile temp_log;
	QTextStream *temp_stream;

	void Write(QStringList);
	public:
	ParupaintRecordManager(QString);
	~ParupaintRecordManager();

	void Join(int);
	void Leave(int);
	void Name(int, QString);
	void Pos(int, int, int, double, bool);
	void Lf(int, int, int);
	void Tool(int, int);
	void Color(int, QString);
	void Width(int, double);
	void Chat(int, QString);
	void Paste(int l, int f, int x, int y, QString base64_img);

	void Fill(int, int, QString);
	void Resize(int, int, bool);
	void Lfa(int, int, QString, QVariant);
	void Lfc(int, int, int, int, bool);
	// TODO lf creation

	void Reset();
};

#endif
