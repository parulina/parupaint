#ifndef PARUPAINTCANVASBRUSH_H
#define PARUPAINTCANVASBRUSH_H

#include <QPixmap>
#include <QColor>
#include <QPen>

#include <QGraphicsItem>

#include "core/parupaintBrush.h"

// todo: extends ParupaintBrush?
class ParupaintCanvasBrush : public QGraphicsItem, public ParupaintBrush
{
	private:
	QRgb current_col;
	QPixmap current_icons;
	QImage icons;
	public:
	~ParupaintCanvasBrush();
	ParupaintCanvasBrush();
	ParupaintCanvasBrush(ParupaintBrush * brush);

	void Paint(QPainter *);
	virtual void SetPosition(QPointF);
	void UpdateIcon();

	private:
	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

};

#endif
