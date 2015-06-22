
#include <QPainter>
#include <QPalette>
#include <QPen>
#include "parupaintCanvasBrush.h"
#include "core/parupaintBrush.h"


ParupaintCanvasBrush::~ParupaintCanvasBrush()
{
}

ParupaintCanvasBrush::ParupaintCanvasBrush() : current_col(0), icons(":/resources/icons.png")
{
	this->setZValue(1);
}

void ParupaintCanvasBrush::UpdateIcon()
{
	auto rgb_list = icons.colorTable();
	rgb_list[1] = this->GetColor().rgba();
	icons.setColorTable(rgb_list);

	current_icons = QPixmap::fromImage(icons);
}

void ParupaintCanvasBrush::SetPosition(QPointF pos)
{
	this->ParupaintBrush::SetPosition(pos);
	setPos(pos);
}

void ParupaintCanvasBrush::Paint(QPainter * painter)
{
	painter->save();

	const auto p = this->GetPressure();
	auto w = this->GetWidth();
	// bucket is just one pix
	if(this->GetToolType() == 1) w = 1;

	const QRectF cc((-QPointF(w/2, w/2)), 		QSizeF(w, w));
	const QRectF cp((-QPointF((w*p)/2, (w*p)/2)),	QSizeF(w*p, w*p));
	if(p > 0 && p < 1){
		QPen pen_inner(this->GetColor());
		pen_inner.setCosmetic(true);
		pen_inner.setWidthF(2);
		painter->setCompositionMode(QPainter::CompositionMode_Source);
		painter->setPen(pen_inner);
		painter->drawEllipse(cp); // pressure

		QPen pen_inner_border(Qt::white);
		pen_inner_border.setCosmetic(true);
		pen_inner_border.setWidthF(1);
		painter->setPen(pen_inner_border);
		painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
		painter->drawEllipse(cp);

	}
	QPen pen(Qt::white);
	pen.setCosmetic(true);
	if(this->GetColor().alpha() == 0){
		pen.setStyle(Qt::DashLine);
	}

	if(current_col != this->GetColor().rgba()){
		current_col = this->GetColor().rgba();
		this->UpdateIcon();
	}
	const auto r_h = icons.height();
	painter->drawPixmap(
			QRectF(QPointF(w/2, -w/2 - r_h), QSizeF(r_h, r_h)),
			current_icons,
			QRectF(QPointF(r_h * this->GetToolType(), 0), QSizeF(r_h, r_h)));

	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	painter->drawEllipse(cc); // brush width

	painter->restore();
}


QRectF ParupaintCanvasBrush::boundingRect() const
{
	const float w = this->GetWidth();
	return QRectF(-w/2.0, -w/2.0, w, w);
}

void ParupaintCanvasBrush::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
	painter->save();

	Paint(painter);
	
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setPen(Qt::black);
	painter->drawText(boundingRect(), Qt::AlignCenter, this->GetName());
	
	painter->restore();
}
