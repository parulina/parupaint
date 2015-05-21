#ifndef PARUPAINTCANVASBRUSHPOOL_H
#define PARUPAINTCANVASBRUSHPOOL_H


#include <QString>
#include <QHash>
class ParupaintCanvasBrush;
class ParupaintBrush;

class ParupaintCanvasBrushPool
{
	private:
	QHash<QString, ParupaintCanvasBrush*> Cursors;
	
	public:
	ParupaintCanvasBrushPool();
	void AddCursor(QString, ParupaintCanvasBrush*);
	void RemoveCursor(QString);
	void RemoveCursor(ParupaintCanvasBrush*);

	ParupaintCanvasBrush * GetCursor(QString);
	ParupaintCanvasBrush * GetCursor(ParupaintBrush*);
	void ClearCursors();
};

#endif

