
#include "parupaintBrushGlass.h"

ParupaintBrushGlass::ParupaintBrushGlass() : brushptr(nullptr), currentBrush(0)
{
	brushes.append(ParupaintBrush("", 3, Qt::black));
	brushes.append(ParupaintBrush("", 30, Qt::white));
	SetBrush(0);
}

ParupaintBrush * ParupaintBrushGlass::GetCurrentBrush()
{
	return brushptr;
}

void ParupaintBrushGlass::ToggleBrush(int from, int to)
{
	if(currentBrush == from){
		SetBrush(to);
	} else {
		SetBrush(from);
	}
}

void ParupaintBrushGlass::SetBrush(int brush)
{
	if(brush <= brushes.size()){
		currentBrush = brush;
		brushptr = &brushes[brush];

	}
}
