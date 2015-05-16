#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include <QGraphicsScene>
class ParupaintCanvasBrush;
class ParupaintCanvasPanvas;

class ParupaintCanvasPool : public QGraphicsScene
{
Q_OBJECT
	private:
	QHash<int, ParupaintCanvasBrush*> Cursors;
	ParupaintCanvasPanvas * Canvas;

//	CurrentFrame
	public:
	ParupaintCanvasPool(QObject *parent);
/*
	public:
	QRectF boundingRect() const;

	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
*/
};





#endif
