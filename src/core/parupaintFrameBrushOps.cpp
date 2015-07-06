
#include "parupaintFrameBrushOps.h"
#include "parupaintFrame.h"
#include "parupaintLayer.h"
#include "parupaintPanvas.h"
#include "parupaintBrush.h"

#include <QtMath>
#include <QDebug>

QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, float ox, float oy, float nx, float ny, ParupaintBrush * brush)
{
	ParupaintLayer * layer = panvas->GetLayer(brush->GetLayer());
	if(!layer) return QRect();

	ParupaintFrame * frame = layer->GetFrame(brush->GetFrame());
	if(!frame) return QRect();

	auto width = brush->GetPressureWidth();
	auto color = brush->GetColor();
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
			QColor d_col = brush->GetColor();
			//const float dot_width = 1 + (brush->GetPressure() > 0.75 ? brush->GetPressure() * 2 : 0);
			const float dot_width = 1;

			for(int x = 0; x < width; x++){
				for(int y = 0; y < width/2; y++){
					const double aa = (double(x) / width) * M_PI;
					const int ax = int(nx) + qRound(cos(aa) * width/2);
					const int ay = int(ny) + qRound(sin(aa) * y);
					const int ay2 = int(ny) - qRound(sin(aa) * y);
					// every nth pixel
					const int m = 4; // + (brush->GetPressure() > 0.75 ? brush->GetPressure() * 2 : 0);

					if(ax % m == 0){
						if(ay % m == 0)
							frame->DrawStep(ax, ay, ax, ay, dot_width, d_col);
						if(ay2 % m == 0)
							frame->DrawStep(ax, ay2, ax, ay2, dot_width, d_col);
					}
				}
			}
			return urect;
		}

		// carries on to default
		// ternarny check because if alpha is 0 it assumed we're erasing
		case 3: color.setAlphaF((brush->GetPressure() <= 0.1 ? 0.1 : brush->GetPressure())*color.alphaF());
		default:
		{
			frame->DrawStep(ox, oy, nx, ny, width, color);
			return urect;
		}
	}
	return QRect();
}
