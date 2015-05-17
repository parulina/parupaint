#ifndef PARUPAINTCURSORPOOL_H
#define PARUPAINTCURSORPOOL_H


#include <QString>
#include <QHash>
class ParupaintCursor;

class ParupaintCursorPool
{
	private:
	QHash<QString, ParupaintCursor*> Cursors;
	
	public:
	ParupaintCursorPool();
	void AddCursor(QString, ParupaintCursor*);
	void RemoveCursor(QString);
	ParupaintCursor * GetCursor(QString str);
	void ClearCursors();
};

#endif

