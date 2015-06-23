#ifndef PARUPAINTCANVASBRUSH_H
#define PARUPAINTCANVASBRUSH_H

#include <QPixmap>
#include <QColor>
#include <QPen>

#include <QGraphicsItem>

#include "core/parupaintBrush.h"

// todo: extends ParupaintBrush?
class ParupaintCanvasBrush : public QGraphicsObject, public ParupaintBrush
{
Q_OBJECT
	Q_PROPERTY(qreal LabelHeight READ nameLabelHeight WRITE setNameLabelHeight)

	protected:
	qreal label_height;
	void setNameLabelHeight(qreal);
	qreal nameLabelHeight() const;

	private:
	QRgb current_col;
	QPixmap current_icons;
	QImage icons;

	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	public:
	~ParupaintCanvasBrush();
	ParupaintCanvasBrush();
	ParupaintCanvasBrush(ParupaintBrush * brush);

	void Paint(QPainter *);
	virtual void SetPosition(QPointF);
	void UpdateIcon();
	void ShowName(double time = -1);

};

#endif
