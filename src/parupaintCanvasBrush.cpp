
#include <QPainter>
#include <QPen>
#include "parupaintCanvasBrush.h"


ParupaintCanvasBrush::ParupaintCanvasBrush()
{

}

void ParupaintCanvasBrush::SetName(QString str)
{
	name = str;
}

void ParupaintCanvasBrush::SetSize(float s)
{
	if(s <= 0) s = 0.1;
	else if(s >= 512) s = 512;
	size = s;
}
void ParupaintCanvasBrush::SetColor(QColor col)
{
	color = col;
}
QPen ParupaintCanvasBrush::ToPen()
{
	QPen pen(color);
	pen.setWidth(size);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	return pen;
}

float ParupaintCanvasBrush::GetSize() const
{
	return size;
}

QColor ParupaintCanvasBrush::GetColor() const
{
	return color;
}

void ParupaintCanvasBrush::Paint(QPainter * painter, QPointF pos, float pressure)
{
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, false);

	QPen pen(Qt::white);
	pen.setCosmetic(true);

	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	
	const QRectF cc((pos - QPointF(size/2, size/2)), QSizeF(size, size));
	const QRectF cp((pos - QPointF((size*pressure)/2, (size*pressure)/2)),
				QSizeF(size*pressure, size*pressure));
	painter->drawEllipse(cc); // brush width

	if(pressure > 0){
		QPen pen_inner(GetColor());
		pen_inner.setCosmetic(true);
		pen_inner.setWidthF(2);
		painter->setCompositionMode(QPainter::CompositionMode_Source);
		painter->setPen(pen_inner);
		painter->drawEllipse(cp); // pressure
	}

	painter->restore();
}
