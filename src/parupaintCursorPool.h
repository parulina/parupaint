#ifndef PARUPAINTCURSORPOOL_H
#define PARUPAINTCURSORPOOL_H


#include <QString>
#include <QHash>
class ParupaintCursor;
class ParupaintBrush;

class ParupaintCursorPool
{
	private:
	QHash<QString, ParupaintCursor*> Cursors;
	
	public:
	ParupaintCursorPool();
	void AddCursor(QString, ParupaintCursor*);
	void RemoveCursor(QString);
	void RemoveCursor(ParupaintCursor*);

	ParupaintCursor * GetCursor(QString);
	ParupaintCursor * GetCursor(ParupaintBrush*);
	void ClearCursors();
};

#endif

