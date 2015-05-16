
#include <QPen>
#include "parupaintCanvasBrush.h"


ParupaintCanvasBrush::ParupaintCanvasBrush()
{

}

void ParupaintCanvasBrush::SetName(QString & str)
{
	name = str;
}

void ParupaintCanvasBrush::SetSize(float s)
{
	if(s <= 0) s = 0.1;
	else if(s >= 512) s = 512;
	size = s;
}
void ParupaintCanvasBrush::SetColor(QColor & col)
{
	color = col;
}
QPen ParupaintCanvasBrush::ToPen()
{
	QPen pen(color);
	pen.setWidth(size);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	return pen;
}

float ParupaintCanvasBrush::Size()
{
	return size;
}
