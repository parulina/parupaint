
#include <QFontMetrics>
#include <QPainter>
#include <QPalette>
#include <QPen>
#include <QPropertyAnimation>
#include <QDebug>

#include "parupaintCanvasBrush.h"
#include "core/parupaintBrush.h"


ParupaintCanvasBrush::~ParupaintCanvasBrush()
{
}

ParupaintCanvasBrush::ParupaintCanvasBrush() : current_col(0), icons(":/resources/icons.png")
{
	this->setZValue(1);
	this->setNameLabelHeight(0);
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
	const auto p = this->GetPressure();
	auto w = this->GetWidth();
	// bucket is just one pix
	if(this->GetToolType() == 1) w = 1;

	const QRectF cc((-QPointF(w/2, w/2)), 		QSizeF(w, w));
	const QRectF cp((-QPointF((w*p)/2, (w*p)/2)),	QSizeF(w*p, w*p));
	if(w > 1 && p > 0 && p < 1){
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

	// Draw icon
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	if(current_col != this->GetColor().rgba()){
		current_col = this->GetColor().rgba();
		this->UpdateIcon();
	}
	const auto r_h = icons.height();
	painter->drawPixmap(
			QRectF(QPointF(w/2, -w/2 - r_h), QSizeF(r_h, r_h)),
			current_icons,
			QRectF(QPointF(r_h * this->GetToolType(), 0), QSizeF(r_h, r_h)));


	// Draw outline
	QPen pen(Qt::white);
	pen.setCosmetic(true);
	if(this->GetColor().alpha() == 0){
		pen.setStyle(Qt::DashLine);
	}
	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);

	if(w > 1) painter->drawEllipse(cc); // round brush width
	else if(w <= 1) {
		painter->drawRect(QRectF(-QPointF(0.5, 0.5), QSizeF(1, 1))); // round brush width

		// Draw some crosshair
		const qreal ch_size = 5;
		painter->drawLine(QLineF(0, -ch_size, 0, -2));
		painter->drawLine(QLineF(0, ch_size, 0, 2));
		painter->drawLine(QLineF(-ch_size, 0, -2, 0));
		painter->drawLine(QLineF(ch_size, 0, 2, 0));
	}
}


QRectF ParupaintCanvasBrush::boundingRect() const
{
	// have a minimum of 250 workable area (for label etc)
	const qreal min = 250;
	float w = this->GetWidth() < min ? min : this->GetWidth();
	return QRectF(-w/2.0, -w/2.0, w, w);
}

void ParupaintCanvasBrush::setNameLabelHeight(qreal h)
{
	this->label_height = h;
	this->update();
}

qreal ParupaintCanvasBrush::nameLabelHeight() const
{
	return label_height;
}

void ParupaintCanvasBrush::ShowName(double time)
{
	if(time < 0){
		this->setNameLabelHeight(1);
		return;
	}
	auto *anim = new QPropertyAnimation(this, "LabelHeight");
	anim->setDuration(time);
	anim->setKeyValueAt(0.0, 1.0);
	anim->setKeyValueAt(0.9, 1.0);
	anim->setKeyValueAt(1.0, 0.0);
	anim->start();
}

void ParupaintCanvasBrush::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	Paint(painter);
	
	painter->setRenderHint(QPainter::Antialiasing, false);
	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter->setPen(Qt::white);

	auto tm = painter->fontMetrics();
	const auto str = this->GetName();

	auto ts = tm.size(Qt::TextSingleLine, str);
	ts.setHeight(ts.height() * this->nameLabelHeight());

	if(ts.width() > this->boundingRect().width()) ts.setWidth(this->boundingRect().width());
	const QRect rrr(QPoint(-ts.width()/2, 0), ts);

	painter->fillRect(rrr, QColor::fromHsl(this->GetColor().hslHue(), 127, 75, 230));
	painter->drawText(rrr, Qt::AlignCenter | Qt::AlignBottom, this->GetName());
}

