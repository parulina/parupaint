
#include "parupaintFrameBrushOps.h"
#include "parupaintFrame.h"
#include "parupaintBrush.h"

#include <QDebug>

QRect ParupaintFrameBrushOps::stroke(float x, float y, float nx, float ny, ParupaintBrush * brush, ParupaintFrame * frame)
{
	if(!frame) return QRect();

	switch(brush->GetToolType()){
		case 1:
		{
			frame->Fill(x, y, brush->GetColor());
			return frame->GetImage().rect();
		}
		default:
		{
			auto width = brush->GetWidth();
			frame->DrawStep(x, y, nx, ny, brush->GetWidth(), brush->GetColor());
			QRect urect(x - width, y - width, x + width, y + width);
			urect |= QRect(nx - width, ny - width, nx + width, ny + width);
			return urect;
		}
	}
	return QRect();
}
