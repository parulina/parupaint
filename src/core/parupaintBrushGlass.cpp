
#include "parupaintBrushGlass.h"

ParupaintBrushGlass::ParupaintBrushGlass() : brushptr(nullptr), currentBrush(0)
{
	// this needs fromHslF, don't remove it.
	// otherwise brush->color.toHsl produces negative hue() and it messes up everything.
	// Qt bug perhaps?
	brushes.append(ParupaintBrush("", 3, QColor::fromHslF(0, 0, 0, 1)));
	brushes.append(ParupaintBrush("", 30, QColor::fromHslF(0, 0, 1, 1)));
	SetBrush(0);
}

ParupaintBrush * ParupaintBrushGlass::GetCurrentBrush()
{
	return brushptr;
}

void ParupaintBrushGlass::ToggleBrush(int from, int to)
{
	auto * brush = this->GetCurrentBrush();

	if(currentBrush == from){
		SetBrush(to);
	} else {
		SetBrush(from);
	}
	this->GetCurrentBrush()->SetDrawing(brush->IsDrawing());
	this->GetCurrentBrush()->SetPosition(brush->GetPosition());
}

void ParupaintBrushGlass::SetBrush(int brush)
{
	if(brush <= brushes.size()){
		currentBrush = brush;
		brushptr = &brushes[brush];

	}
}
