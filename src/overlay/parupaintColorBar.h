#ifndef PARUPAINTCOLORBAR_H
#define PARUPAINTCOLORBAR_H

#include <QSlider>

enum ColorBarType {
	COLOR_BAR_TYPE_ALPHA = 1,
	COLOR_BAR_TYPE_SATUR,
	COLOR_BAR_TYPE_LIGHT,
};

class ParupaintColorBar : public QSlider
{
Q_OBJECT
	private:
	ColorBarType bartype;
	qreal additional_value;

	void paintEvent(QPaintEvent * event);

	public:
	ParupaintColorBar(ColorBarType type, QWidget * = nullptr);

	void setRealValue(qreal);
	qreal getRealValue();

	signals:
	void valueChangedF(qreal);
};

#endif
