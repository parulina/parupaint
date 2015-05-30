#ifndef PARUPAINTCOLORPICKER_H
#define PARUPAINTCOLORPICKER_H

#include "parupaintOverlayWidget.h"
class Color_Wheel;

class ParupaintColorPicker : public ParupaintOverlayWidget
{
Q_OBJECT
	private:
	Color_Wheel * wheel;

	void wheelEvent(QWheelEvent*);

	public:
	ParupaintColorPicker(QWidget * parent = nullptr);
	void SetColor(QColor);

	signals:
	void ColorChange(QColor);

};

#endif
