
#include "parupaintColorPicker.h"
#include "../qtcolorpicker/Color_Wheel"

#include <QDebug>
#include <QWheelEvent>

ParupaintColorPicker::ParupaintColorPicker(QWidget * parent) : ParupaintOverlayWidget(parent)
{
	// TODO better this?
	this->setFocusPolicy(Qt::NoFocus);
	this->setObjectName("ColorPicker");
	resize(200, 200);

	wheel = new Color_Wheel(this);
	wheel->resize(200, 200);
	wheel->setDisplayFlag(Color_Wheel::ANGLE_FIXED, Color_Wheel::ANGLE_FLAGS);
	wheel->setFocusPolicy(Qt::NoFocus);

	connect(wheel, &Color_Wheel::colorChanged, this, &ParupaintColorPicker::ColorChange);
}

void ParupaintColorPicker::SetColor(QColor c)
{
	wheel->setColor(c);
}

void ParupaintColorPicker::wheelEvent(QWheelEvent * event)
{
	auto y = event->angleDelta().y()/120;
	auto h = wheel->hue() + y/60.0;
	if(h > 1) 	h -= 1;
	else if(h < 0)	h += 1;

	wheel->setHue(h);
	emit ColorChange(wheel->color());
	QWidget::wheelEvent(event);
}

