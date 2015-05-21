
#include <QPainter>
#include <QPen>
#include "parupaintCanvasBrush.h"
#include "core/parupaintBrush.h"


ParupaintCanvasBrush::ParupaintCanvasBrush(ParupaintBrush * b) : brush(b)
{
	this->setZValue(1);
}

void ParupaintCanvasBrush::SetPosition(QPointF pos)
{
	brush->SetPosition(pos);
	setPos(pos);
}
void ParupaintCanvasBrush::SetWidth(float f) 
{
	brush->SetWidth(f);
}

void ParupaintCanvasBrush::SetPressure(float f) 
{
	brush->SetPressure(f);
}

void ParupaintCanvasBrush::SetColor(QColor col) 
{
	brush->SetColor(col);
}

QPointF ParupaintCanvasBrush::GetPosition() const
{
	return brush->GetPosition();
}

float ParupaintCanvasBrush::GetWidth() const
{
	return brush->GetWidth();
}
float ParupaintCanvasBrush::GetPressure() const
{
	return brush->GetPressure();
}

QColor ParupaintCanvasBrush::GetColor() const
{
	return brush->GetColor();
}

ParupaintBrush * ParupaintCanvasBrush::GetBrush() const
{
	return brush;
}
void ParupaintCanvasBrush::Paint(QPainter * painter)
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


QRectF ParupaintCanvasBrush::boundingRect() const
{
	const float w = brush->GetWidth();
	return QRectF(-w/2.0, -w/2.0, w, w);
}

void ParupaintCanvasBrush::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
	painter->save();

	Paint(painter);
	
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, brush->GetName());
	
	painter->restore();
}
