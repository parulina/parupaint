#ifndef PARUPAINTCANVASOBJECT_H
#define PARUPAINTCANVASOBJECT_H

#include <QGraphicsObject>

class ParupaintPanvas;

class ParupaintCanvasObject : public QGraphicsObject
{
Q_OBJECT
	private:
	ParupaintPanvas * Canvas;

	public:
	ParupaintCanvasObject(QGraphicsItem *parent=0);
	
	
	public:
	QRectF boundingRect() const;

	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
};

#endif
