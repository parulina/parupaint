#ifndef PARUPAINTCANVASSTROKEOBJECT_H
#define PARUPAINTCANVASSTROKEOBJECT_H

#include "stroke/parupaintStroke.h"
#include <QGraphicsItem>

class ParupaintCanvasObject;

class ParupaintCanvasStrokeObject : public QGraphicsItem, public ParupaintStroke
{
	private:
	QRectF canvasRectangle;

	public:
	ParupaintCanvasStrokeObject(ParupaintCanvasObject * = nullptr);


	private:
	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
};

#endif
