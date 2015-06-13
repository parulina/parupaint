
#include "parupaintColorBar.h"

#include <QStyle>
#include <QPainter>
#include <QLinearGradient>
#include <QProxyStyle>

#include <QDebug>

class BarStyle : public QProxyStyle
{
	public:
	BarStyle(const QString& baseStyle) : QProxyStyle(baseStyle) { }
	BarStyle(QStyle* baseStyle) : QProxyStyle(baseStyle) { }

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_Slider_AbsoluteSetButtons) {
			return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
		}
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

ParupaintColorBar::ParupaintColorBar(ColorBarType type, QWidget * parent) : 
	QSlider(parent), bartype(type), additional_value(0.0)
{
	this->setObjectName("ColorBar");
	if(type == COLOR_BAR_TYPE_SATUR){
		this->setOrientation(Qt::Horizontal);
	} else {
		this->setOrientation(Qt::Vertical);
	}
	this->setMaximum(255);
	this->setFocusPolicy(Qt::NoFocus);
	this->setStyle(new BarStyle(this->style()));
	connect(this, &QSlider::valueChanged, [=](int r){
		emit valueChangedF(qreal(r) / qreal(this->maximum()));
	});
}

void ParupaintColorBar::setRealValue(qreal v)
{
	additional_value = v;
}

qreal ParupaintColorBar::getRealValue()
{
	return qreal(qreal(this->value()) / qreal(this->maximum()));
}

void ParupaintColorBar::paintEvent(QPaintEvent * event)
{
	QPainter paint(this);
	if(bartype == COLOR_BAR_TYPE_ALPHA){

		paint.fillRect(this->rect(), QColor::fromRgbF(1, 1, 1, this->getRealValue()));

	} else if(bartype == COLOR_BAR_TYPE_SATUR){
		// value is hue
		QLinearGradient gradient(this->rect().bottomLeft(), this->rect().topRight());
		gradient.setColorAt(0, QColor::fromHslF(additional_value, 0, 0.5));
		gradient.setColorAt(1, QColor::fromHslF(additional_value, 1, 0.5));

		paint.fillRect(this->rect(), gradient);

	} else if(bartype == COLOR_BAR_TYPE_LIGHT){
		// value is hue
		QLinearGradient gradient(this->rect().topLeft(), this->rect().bottomRight());
		gradient.setColorAt(0, QColor::fromHslF(additional_value, 1, 1));
		gradient.setColorAt(0.5, QColor::fromHslF(additional_value, 1, 0.5));
		gradient.setColorAt(1, QColor::fromHslF(additional_value, 1, 0));

		paint.fillRect(this->rect(), gradient);
	}

	this->QSlider::paintEvent(event);
}
