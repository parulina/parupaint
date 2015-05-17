
#include <QPainter>
#include <QPen>
#include "parupaintBrush.h"


ParupaintBrush::ParupaintBrush()
{
	SetName("Null");
	SetPressure(0.0);
	SetWidth(10);
}

void ParupaintBrush::SetName(QString str)
{
	name = str;
}

void ParupaintBrush::SetWidth(float w)
{
	if(w <= 0) w = 0.1;
	else if(w >= 512) w = 512;
	width = w;
}
void ParupaintBrush::SetPressure(float p)
{
	pressure = p;
}

void ParupaintBrush::SetColor(QColor col)
{
	color = col;
}
void ParupaintBrush::SetPosition(QPointF pos)
{
	position = pos;
}

float ParupaintBrush::GetWidth() const
{
	return width;
}
float ParupaintBrush::GetPressure() const
{
	return pressure;
}

QColor ParupaintBrush::GetColor() const
{
	return color;
}

QString ParupaintBrush::GetName() const
{
	return name;
}

QPointF ParupaintBrush::GetPosition() const
{
	return position;
}
