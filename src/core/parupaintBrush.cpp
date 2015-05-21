
#include <QPainter>
#include <QPen>
#include "parupaintBrush.h"


ParupaintBrush::ParupaintBrush()
{
	SetName("");
	SetPressure(0.0);
	SetWidth(1);
	SetCurrentStroke(nullptr);
	SetLastStroke(nullptr);
	SetLayer(0);
	SetFrame(0);
	SetDrawing(false);
}

QPen ParupaintBrush::ToPen()
{
	QPen pen(GetColor());
	pen.setWidth(GetWidth()*GetPressure());
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	return pen;
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
void ParupaintBrush::SetPosition(float x, float y)
{
	position = QPointF(x, y);
}
void ParupaintBrush::SetPosition(QPointF pos)
{
	position = pos;
}

void ParupaintBrush::SetCurrentStroke(ParupaintStroke * s)
{
	CurrentStroke = s;
}
void ParupaintBrush::SetLastStroke(ParupaintStroke * s)
{
	LastStroke = s;
}

void ParupaintBrush::SetLayer(_lint l)
{
	layer = l;
}
void ParupaintBrush::SetFrame(_fint f)
{
	frame = f;
}

void ParupaintBrush::SetDrawing(bool d)
{
	drawing = d;
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

_lint ParupaintBrush::GetLayer() const 
{
	return layer;
}

_fint ParupaintBrush::GetFrame() const
{
	return frame;
}

bool ParupaintBrush::IsDrawing() const
{
	return drawing;
}

ParupaintStroke * ParupaintBrush::GetCurrentStroke() const
{
	return CurrentStroke;
}
ParupaintStroke * ParupaintBrush::GetLastStroke() const
{
	return LastStroke;
}
