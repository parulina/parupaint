#ifndef PARUPAINTCANVASBRUSH_H
#define PARUPAINTCANVASBRUSH_H

#include <QColor>
#include <QPen>

#include <QGraphicsItem>

class ParupaintBrush;

// todo: extends ParupaintBrush?
class ParupaintCursor : public QGraphicsItem 
{
	private:
	ParupaintBrush * brush;
	public:
	ParupaintCursor(ParupaintBrush *);

	void Paint(QPainter *);

	void SetPosition(QPointF);
	void SetWidth(float);
	void SetPressure(float);
	void SetColor(QColor);

	QPointF GetPosition() const;
	float GetWidth() const;
	float GetPressure() const;
	QColor GetColor() const;

	ParupaintBrush * GetBrush() const;

	private:
	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

};

#endif
