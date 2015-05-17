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
	ParupaintCanvasObject();
	QRectF boundingRect() const;
	
	// Extend the resize to send signals, too.
	virtual void Resize(QSize s);

	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	signals:
	void ResizeSignal(QSize old_size, QSize new_size);
};

#endif
