#ifndef PARUPAINTCOLORPICKER_H
#define PARUPAINTCOLORPICKER_H

#include <QFrame>

class QVBoxLayout;
class QHBoxLayout;

class ParupaintColorBar;
class ParupaintColorWheel;

class ParupaintColorPicker : public QFrame
{
Q_OBJECT

	private:
	ParupaintColorBar * alpha_slider;
	ParupaintColorBar * light_slider;
	ParupaintColorBar * saturation_slider;
	ParupaintColorWheel * hue_wheel;

	QVBoxLayout * main_vlayout;
	QHBoxLayout * ahs_hlayout;

	QColor preview_color;
	void paintEvent(QPaintEvent*);

	public:
	ParupaintColorPicker(QWidget * parent = nullptr);
	QSize sizeHint() const;

	void SetColor(QColor);

	void SetHue(qreal);
	void SetSat(qreal);
	void SetLit(qreal);
	void SetAlp(qreal);

	public slots:
	void color_change(QColor);

	signals:
	void ColorChange(QColor);

};

#endif
