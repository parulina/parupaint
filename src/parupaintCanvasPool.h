#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include <QGraphicsScene>
class ParupaintCanvasBrush;
class ParupaintCanvasObject;


class ParupaintCanvasPool : public QGraphicsScene
{
Q_OBJECT
	private:
	QHash<int, ParupaintCanvasBrush*> Cursors;
	ParupaintCanvasObject * Canvas;

//	CurrentFrame
	public:
	ParupaintCanvasPool(QObject *parent);
	void ClearCursors();
};





#endif
