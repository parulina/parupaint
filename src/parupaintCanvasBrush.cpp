
#include <QFontMetrics>
#include <QGraphicsScene>
#include <QCursor>
#include <QPainter>
#include <QTransform>
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
	if(this->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill) w = 1;

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


	const QTransform & t = painter->transform();
	const qreal zoom_x = t.m11(), zoom_y = t.m22();
	if(zoom_y < 1) painter->setTransform(QTransform(1, t.m12(), t.m13(), t.m21(), 1, t.m23(), t.m31(), t.m32(), t.m33()));

	const auto r_h = icons.height();
	painter->drawPixmap(
			QRectF(QPointF(0, - r_h), QSizeF(r_h, r_h)),
			current_icons,
			QRectF(QPointF(r_h * this->GetToolType(), 0), QSizeF(r_h, r_h)));

	if(zoom_y < 1) painter->setTransform(QTransform(zoom_x, t.m12(), t.m13(), t.m21(), zoom_y, t.m23(), t.m31(), t.m32(), t.m33()));

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
	qreal min_w = 100, min_h = 30;

	if(this->GetWidth() > min_h) min_h = this->GetWidth();
	if(this->GetWidth() > min_w) min_w = this->GetWidth();

	return QRectF(-min_w/2.0, -min_h/2.0, min_w, min_h);
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
	painter->setRenderHint(QPainter::Antialiasing, false);

	Paint(painter);

	painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
	painter->setPen(Qt::white);

	const QTransform & t = painter->transform();
	const qreal zoom_x = t.m11(), zoom_y = t.m22();

	if(this->nameLabelHeight() > 0) {
		QRectF bound_rect = this->boundingRect();
		QRectF text_rect(0, 0, 0, 0);

		QSize text_size = painter->fontMetrics().size(Qt::TextSingleLine, this->GetName());
		text_rect.setWidth(text_size.width() > bound_rect.width() ? bound_rect.width() : text_size.width());
		text_rect.setHeight((bound_rect.height()/2 - 1) * this->nameLabelHeight());
		text_rect.setRect(-(text_rect.width()/2.0), -(zoom_y < 1 ? (text_rect.height()/2.0) : 0),
				text_rect.width(), text_rect.height());

		if((text_rect.height() / zoom_y) < bound_rect.height()) {
			painter->save();

			painter->setTransform(QTransform(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33()));
			if(this->isUnderMouse() && (text_rect.height() / zoom_y > 2)) painter->setOpacity(0.5);

			painter->fillRect(text_rect, QColor::fromHsl(this->GetColor().hslHue(), 127, 75, 230));
			painter->drawText(text_rect, Qt::AlignCenter | Qt::AlignBottom, this->GetName());

			painter->restore();
		}
	}
}

