
#include "parupaintFrameBrushOps.h"
#include "parupaintFrame.h"
#include "parupaintBrush.h"

#include <QDebug>

QRect ParupaintFrameBrushOps::stroke(float ox, float oy, float nx, float ny, ParupaintBrush * brush, ParupaintFrame * frame)
{
	if(!frame) return QRect();

	auto width = brush->GetWidth();
	QRect urect(ox - width, oy - width, ox + width, oy + width);
	urect |= QRect(nx - width, ny - width, nx + width, ny + width);
	switch(brush->GetToolType()){
		case 1:
		{
			frame->Fill(nx, ny, brush->GetColor());
			return frame->GetImage().rect();
		}
		case 2:
		{
			for(int x = 0; x < width; x++){
				for(int y = 0; y < width; y++){
					const int ax = int(nx) + x;
					const int ay = int(ny) + y;
					const int m = 5;

					if(ax % m == 0 && ay % m == 0){
						frame->DrawStep(ax, ay, ax, ay, 1, brush->GetColor());
					}
				}
			}
			return urect;
		}
		default:
		{
			frame->DrawStep(ox, oy, nx, ny, brush->GetWidth(), brush->GetColor());
			return urect;
		}
	}
	return QRect();
}
