
#include <QPainter>
#include <QPen>
#include "parupaintCursor.h"
#include "parupaintBrush.h"


ParupaintCursor::ParupaintCursor(ParupaintBrush * b) : brush(b)
{
	this->setZValue(1);
}

void ParupaintCursor::SetPosition(QPointF pos)
{
	brush->SetPosition(pos);
	setPos(pos);
}
void ParupaintCursor::SetWidth(float f) 
{
	brush->SetWidth(f);
}

void ParupaintCursor::SetPressure(float f) 
{
	brush->SetPressure(f);
}

void ParupaintCursor::SetColor(QColor col) 
{
	brush->SetColor(col);
}

QPointF ParupaintCursor::GetPosition() const
{
	return brush->GetPosition();
}

float ParupaintCursor::GetWidth() const
{
	return brush->GetWidth();
}
float ParupaintCursor::GetPressure() const
{
	return brush->GetPressure();
}

QColor ParupaintCursor::GetColor() const
{
	return brush->GetColor();
}

ParupaintBrush * ParupaintCursor::GetBrush() const
{
	return brush;
}

QPen ParupaintCursor::ToPen()
{
	QPen pen(brush->GetColor());
	pen.setWidth(brush->GetWidth());
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	return pen;
}
void ParupaintCursor::Paint(QPainter * painter)
{
	painter->save();

	painter->setRenderHint(QPainter::Antialiasing, false);

	QPen pen(Qt::white);
	pen.setCosmetic(true);

	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	
	const auto w = brush->GetWidth();
	const auto p = brush->GetPressure();

	const QRectF cc((-QPointF(w/2, w/2)), 		QSizeF(w, w));
	const QRectF cp((-QPointF((w*p)/2, (w*p)/2)),	QSizeF(w*p, w*p));
	painter->drawEllipse(cc); // brush width

	if(p > 0){
		QPen pen_inner(brush->GetColor());
		pen_inner.setCosmetic(true);
		pen_inner.setWidthF(2);
		painter->setCompositionMode(QPainter::CompositionMode_Source);
		painter->setPen(pen_inner);
		painter->drawEllipse(cp); // pressure
	}

	painter->restore();
}


QRectF ParupaintCursor::boundingRect() const
{
	const float w = brush->GetWidth();
	return QRectF(-w/2.0, -w/2.0, w, w);
}

void ParupaintCursor::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
	painter->save();

	Paint(painter);
	
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, brush->GetName());
	
	painter->restore();
}
