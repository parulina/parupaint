#ifndef PARUPAINTCANVASBRUSH_H
#define PARUPAINTCANVASBRUSH_H

#include <QColor>
#include <QPen>

#include <QGraphicsItem>

#include "core/parupaintBrush.h"

// todo: extends ParupaintBrush?
class ParupaintCanvasBrush : public QGraphicsItem, public ParupaintBrush
{
	public:
	~ParupaintCanvasBrush();
	ParupaintCanvasBrush();
	ParupaintCanvasBrush(ParupaintBrush * brush);

	void Paint(QPainter *);
	virtual void SetPosition(QPointF);

	private:
	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

};

#endif
