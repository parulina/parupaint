#include "parupaintColorPicker.h"

#include <QDebug>
#include <QWheelEvent>
#include <QPainter>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "parupaintColorBar.h"
#include "parupaintColorWheel.h"

ParupaintColorPicker::ParupaintColorPicker(QWidget * parent) : QFrame(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);

	main_vlayout = new QVBoxLayout;
	main_vlayout->setMargin(0);
	main_vlayout->setSpacing(0);

	// alpha hue saturation
	ahs_hlayout = new QHBoxLayout;
	ahs_hlayout->setMargin(0);

		alpha_slider = new ParupaintColorBar(COLOR_BAR_TYPE_ALPHA);
		ahs_hlayout->addWidget(alpha_slider, Qt::AlignLeft);
		connect(alpha_slider, &ParupaintColorBar::valueChangedF, this, &ParupaintColorPicker::SetAlp);

		ahs_hlayout->addStretch();

		hue_wheel = new ParupaintColorWheel;
		ahs_hlayout->addWidget(hue_wheel, Qt::AlignCenter);
		connect(hue_wheel, &ParupaintColorWheel::valueChangedF, this, &ParupaintColorPicker::SetHue);

		ahs_hlayout->addStretch();

		light_slider = new ParupaintColorBar(COLOR_BAR_TYPE_LIGHT);
		ahs_hlayout->addWidget(light_slider, Qt::AlignRight);
		connect(light_slider, &ParupaintColorBar::valueChangedF, this, &ParupaintColorPicker::SetLit);

	main_vlayout->addLayout(ahs_hlayout);
	
	saturation_slider = new ParupaintColorBar(COLOR_BAR_TYPE_SATUR);
	connect(saturation_slider, &ParupaintColorBar::valueChangedF, this, &ParupaintColorPicker::SetSat);
	main_vlayout->addWidget(saturation_slider);

	this->setLayout(main_vlayout);

	this->SetColor(QColor::fromHslF(0, 0.5, 0.5));
}

void ParupaintColorPicker::SetHue(qreal r)
{
	qreal hue = r;
	qreal sat = preview_color.hslSaturationF();
	qreal lit = preview_color.lightnessF();
	qreal alp = preview_color.alphaF();

	preview_color = QColor::fromHslF(hue, sat, lit, alp);

	this->light_slider->setRealValue(hue);
	this->saturation_slider->setRealValue(hue);
	this->update();
	emit ColorChange(preview_color);
}
void ParupaintColorPicker::SetSat(qreal r)
{
	qreal hue = preview_color.hslHueF();
	qreal sat = r;
	qreal lit = preview_color.lightnessF();
	qreal alp = preview_color.alphaF();

	preview_color = QColor::fromHslF(hue, sat, lit, alp);
	this->update();
	emit ColorChange(preview_color);
}
void ParupaintColorPicker::SetLit(qreal r)
{
	qreal hue = preview_color.hslHueF();
	qreal sat = preview_color.hslSaturationF();
	qreal lit = r;
	qreal alp = preview_color.alphaF();

	preview_color = QColor::fromHslF(hue, sat, lit, alp);

	this->update();
	emit ColorChange(preview_color);
}
void ParupaintColorPicker::SetAlp(qreal r)
{
	qreal hue = preview_color.hslHueF();
	qreal sat = preview_color.hslSaturationF();
	qreal lit = preview_color.lightnessF();
	qreal alp = r;

	preview_color = QColor::fromHslF(hue, sat, lit, alp);

	this->update();
	emit ColorChange(preview_color);
}

// slot
void ParupaintColorPicker::color_change(QColor color)
{
	bool signals_blocked = this->blockSignals(true);
		this->SetColor(color);
	this->blockSignals(signals_blocked);
}

void ParupaintColorPicker::paintEvent(QPaintEvent* event)
{
	// draw current color in the middle

	QPainter paint(this);
	
	const int preview_width = 38;
 	QPoint cr = ahs_hlayout->contentsRect().center();

 	QPen pen(preview_color, preview_width);
 	paint.setPen(pen);
 	paint.drawEllipse(cr, preview_width/2, preview_width/2);

	paint.setPen(QPen(Qt::white, 2));
	paint.drawEllipse(cr, preview_width, preview_width);

 	this->QFrame::paintEvent(event);
}

void ParupaintColorPicker::SetColor(QColor c)
{
	preview_color = c.toHsl();

 	alpha_slider->setValue(preview_color.alpha());
 	light_slider->setValue(preview_color.lightness());
  	saturation_slider->setValue(preview_color.hslSaturation());
 	hue_wheel->setValue(preview_color.hslHue());

	auto hue = preview_color.hslHueF();
	light_slider->setRealValue(hue);
	saturation_slider->setRealValue(hue);

	this->update();
}

QSize ParupaintColorPicker::sizeHint() const
{
	return QSize(200, 200);
}
