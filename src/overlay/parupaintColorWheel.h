#ifndef PARUPAINTCOLORWHEEL_H
#define PARUPAINTCOLORWHEEL_H

#include <QDial>

class ParupaintColorWheel : public QDial
{
Q_OBJECT
	private:
	qreal wheel_width;
	void paintEvent(QPaintEvent * event);
	public:
	ParupaintColorWheel(QDial * = nullptr);

	signals:
	void valueChangedF(qreal);
};

#endif
