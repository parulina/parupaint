#ifndef PARUPAINTCOLORPICKER_H
#define PARUPAINTCOLORPICKER_H

#include "parupaintOverlayWidget.h"

class QVBoxLayout;
class QHBoxLayout;

class ParupaintColorBar;
class ParupaintColorWheel;

class ParupaintColorPicker : public ParupaintOverlayWidget
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
	void keyPressEvent(QKeyEvent*);

	public:
	ParupaintColorPicker(QWidget * parent = nullptr);
	void SetColor(QColor);

	void SetHue(qreal);
	void SetSat(qreal);
	void SetLit(qreal);
	void SetAlp(qreal);

	signals:
	void ColorChange(QColor);

};

#endif
