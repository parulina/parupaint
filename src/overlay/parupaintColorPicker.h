#ifndef PARUPAINTCOLORPICKER_H
#define PARUPAINTCOLORPICKER_H

#include "../qtcolorpicker/Color_Wheel"
#include "parupaintOverlayWidget.h"

class ParupaintColorPicker : public ParupaintOverlayWidget
{
	private:
	Color_Wheel * wheel;

	void wheelEvent(QWheelEvent*);

	public:
	ParupaintColorPicker(QWidget * parent = nullptr);



};

#endif
