
#include "parupaintBrushGlass.h"

ParupaintBrushGlass::ParupaintBrushGlass() : brushptr(nullptr), currentBrush(0)
{
	brushes += ParupaintBrush("",  1, Qt::black);
	brushes += ParupaintBrush("", 30, Qt::white);
	brushes += ParupaintBrush("",  3, Qt::gray);
	brushes += ParupaintBrush("", 30, Qt::transparent);
	brushes += ParupaintBrush("",  3, Qt::red);
	brushes += ParupaintBrush("",  3, Qt::green);
	brushes += ParupaintBrush("",  3, Qt::blue);
	SetBrush(0);
}

ParupaintBrush * ParupaintBrushGlass::GetCurrentBrush()
{
	return brushptr;
}

int ParupaintBrushGlass::GetCurrentBrushNum() const
{
	return currentBrush;
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
}

void ParupaintBrushGlass::SetBrush(int brush)
{
	auto * original_brush = this->GetCurrentBrush();
	if(brush <= brushes.size()){
		currentBrush = brush;
		brushptr = &brushes[brush];
		if(original_brush != nullptr) this->GetCurrentBrush()->SetPosition(original_brush->GetPosition());
	}
}
