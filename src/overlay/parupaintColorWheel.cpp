
#include "parupaintColorWheel.h"

#include <QConicalGradient>
#include <QPainter>
#include <QStyle>
#include <QtMath>

#include <QDebug>

ParupaintColorWheel::ParupaintColorWheel(QDial * parent) : QDial(parent), wheel_width(30)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setObjectName("ColorWheel");
	this->setWrapping(true);
	this->setNotchesVisible(false);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setMaximum(360);
	connect(this, &QDial::valueChanged, [=](int r){
		emit valueChangedF(qreal(r) / qreal(this->maximum()));
	});
}


void ParupaintColorWheel::paintEvent(QPaintEvent * event)
{
	// QSS won't do much stuff.
	
	const int max_ratio = (this->rect().width() > this->rect().height() ? this->rect().height() : this->rect().width())/2;
	QPoint rp = QPoint(max_ratio - wheel_width/2, max_ratio - wheel_width/2);
	QRect rr = QRect(this->rect().center() - rp, 
			this->rect().center() + rp);

	QPainter paint(this);
	QConicalGradient gradient;
	gradient.setCenter(this->rect().center());
	gradient.setAngle(-90);
	for(qreal i = 0; i <= 1; i += 0.1){
		gradient.setColorAt(i, QColor::fromHslF(1.0 - i, 1, 0.5));
	}
	QPen pen(QBrush(gradient), wheel_width);
	paint.setPen(pen);

	paint.drawArc(rr, 0, 360 * 16);

	qreal ttf = ((qreal(this->value()) / qreal(this->maximum()))*(M_PI*2) + M_PI/2);
	QPoint pp = this->rect().center() + QPoint(qCos(ttf) * (max_ratio), qSin(ttf) * (max_ratio));

	QPen pen2(Qt::black);
	pen2.setWidth(2);
	paint.setPen(pen2);
	paint.drawLine(this->rect().center(), pp);
	
// 	this->QDial::paintEvent(event);
}
