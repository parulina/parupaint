
#include <QPainter>
#include <QPen>
#include "parupaintCanvasBrush.h"


ParupaintCanvasBrush::ParupaintCanvasBrush()
{

}

void ParupaintCanvasBrush::SetPosition(QPointF pos)
{
	ParupaintBrush::SetPosition(pos);
	setPos(pos);
}

QPen ParupaintCanvasBrush::ToPen()
{
	QPen pen(GetColor());
	pen.setWidth(GetWidth());
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	return pen;
}
void ParupaintCanvasBrush::Paint(QPainter * painter)
{
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, false);

	QPen pen(Qt::white);
	pen.setCosmetic(true);

	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	
	const auto w = GetWidth();
	const auto p = GetPressure();

	const QRectF cc((-QPointF(w/2, w/2)), 		QSizeF(w, w));
	const QRectF cp((-QPointF((w*p)/2, (w*p)/2)),	QSizeF(w*p, w*p));
	painter->drawEllipse(cc); // brush width

	if(p > 0){
		QPen pen_inner(GetColor());
		pen_inner.setCosmetic(true);
		pen_inner.setWidthF(2);
		painter->setCompositionMode(QPainter::CompositionMode_Source);
		painter->setPen(pen_inner);
		painter->drawEllipse(cp); // pressure
	}

	painter->restore();
}


QRectF ParupaintCanvasBrush::boundingRect() const
{
	const float w = GetWidth();
	return QRectF(-w/2.0, -w/2.0, w, w);
}

void ParupaintCanvasBrush::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
	painter->save();

	Paint(painter);
	
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, GetName());
	
	painter->restore();
}
