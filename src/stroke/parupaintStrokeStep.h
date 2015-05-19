#ifndef PARUPAINTSTROKESTEP_H
#define PARUPAINTSTROKESTEP_H

#include "../parupaintBrush.h"


class ParupaintStrokeStep : public ParupaintBrush
{
	public:

	ParupaintStrokeStep(){};
	ParupaintStrokeStep(float x, float y, float w, QColor col) : ParupaintStrokeStep()
	{
		SetPosition(x, y);
		SetWidth(w);
		SetColor(col);
	}
	ParupaintStrokeStep(ParupaintBrush b) : ParupaintStrokeStep()
	{
		this->SetPosition(b.GetPosition());
		this->SetWidth(b.GetWidth());
		this->SetPressure(b.GetPressure());
		this->SetColor(b.GetColor());
	}
};

#endif
