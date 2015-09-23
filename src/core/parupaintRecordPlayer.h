#ifndef PARUPAINTRECORDPLAYER_H
#define PARUPAINTRECORDPLAYER_H

class ParupaintRecordPlayer
{
	QByteArray script;
	int current_line;
	int total_lines;

	bool sleeping;
	bool will_restore;

	public:
	ParupaintRecordPlayer();
	void Reset();
	void LoadFromFile(QFile &);
	bool TakeLine(QString & str);

	void SetSleeping(bool);
	bool IsSleeping() const;
	void SetWillRestore(bool);
	bool WillRestore() const;

	int GetCurrentLine() const;
	int GetTotalLines() const;
	qreal GetProgress() const;
};

#endif
