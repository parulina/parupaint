
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
	SetToolType(0);
}

ParupaintBrush::ParupaintBrush(QString name, double w, QColor col) : ParupaintBrush()
{
	SetName(name);
	SetWidth(w);
	SetColor(col);
}

QPen ParupaintBrush::ToPen()
{
	QPen pen(GetColor());
	if(GetPressure() < 0) 	pen.setWidth(GetWidth());
	else 			pen.setWidth(GetWidth()*GetPressure());
	pen.setCapStyle(Qt::RoundCap);
	return pen;
}

void ParupaintBrush::SetName(QString str)
{
	name = str;
}

void ParupaintBrush::SetWidth(double w)
{
	if(w <= 0) w = 0.1;
	else if(w >= 512) w = 512;
	width = w;
}
void ParupaintBrush::SetPressure(double p)
{
	pressure = p;
}

void ParupaintBrush::SetColor(QColor col)
{
	color = col;
}
void ParupaintBrush::SetPosition(double x, double y)
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

void ParupaintBrush::SetToolType(int t)
{
	tooltype=t;
}

double ParupaintBrush::GetWidth() const
{
	return width;
}
double ParupaintBrush::GetPressure() const
{
	return pressure;
}

QColor ParupaintBrush::GetColor() const
{
	return color;
}

QString ParupaintBrush::GetColorString() const
{
	return ("#"+ 
		("0" + QString::number(color.red(), 16)).right(2) + 
		("0" + QString::number(color.green(), 16)).right(2) + 
		("0" + QString::number(color.blue() , 16)).right(2) +
		("0" + QString::number(color.alpha(), 16)).right(2)).toUpper();
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

int ParupaintBrush::GetToolType() const
{
	return tooltype;
}

double ParupaintBrush::GetPressureWidth() const
{
	return width * pressure;
}

