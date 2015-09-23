
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include "parupaintRecordPlayer.h"

ParupaintRecordPlayer::ParupaintRecordPlayer()
{
	this->Reset();
}

void ParupaintRecordPlayer::Reset()
{
	script.clear();
	current_line = total_lines = 0;
	sleeping = will_restore = false;
}

void ParupaintRecordPlayer::LoadFromFile(QFile & file)
{
	this->Reset();

	script = file.readAll();
	QTextStream stream(script);
	while(!stream.atEnd()){
		stream.readLine(0);
		total_lines++;
	}
}

bool ParupaintRecordPlayer::TakeLine(QString & str)
{
	QTextStream stream(script);
	str = stream.readLine();

	// TODO
	// mess with cursor positions rather than cutting
	// he buffer off? in order to preserve and be able to
	// rollback and stuff
	script = script.mid(stream.pos());

	return (stream.atEnd() && str.isEmpty());
}

void ParupaintRecordPlayer::SetSleeping(bool b)
{
	sleeping = b;
}
bool ParupaintRecordPlayer::IsSleeping() const
{
	return sleeping;
}
void ParupaintRecordPlayer::SetWillRestore(bool b)
{
	will_restore = b;
}
bool ParupaintRecordPlayer::WillRestore() const
{
	return will_restore;
}

int ParupaintRecordPlayer::GetCurrentLine() const
{
	return current_line;
}
int ParupaintRecordPlayer::GetTotalLines() const
{
	return total_lines;
}
qreal ParupaintRecordPlayer::GetProgress() const
{
	return ((qreal)current_line / (qreal)total_lines);
}
