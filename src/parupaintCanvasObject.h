#ifndef PARUPAINTCANVASOBJECT_H
#define PARUPAINTCANVASOBJECT_H

#include "panvas/parupaintPanvas.h"

#include <QGraphicsObject>

// Now put together Panvas and GraphicsObject.
// Is it a good idea?


class ParupaintCanvasObject : public QGraphicsObject, public ParupaintPanvas
{
Q_OBJECT
	public:
	QRectF boundingRect() const;

	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
};

#endif
