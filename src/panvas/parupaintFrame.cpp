
#include <QPainter>
#include <QPen>

#include "parupaintFrame.h"
#include "../stroke/parupaintStrokeStep.h"
#include "../stroke/parupaintStroke.h"

ParupaintFrame::ParupaintFrame() : Extended(false), Opacity(1.0)
{
}

ParupaintFrame::ParupaintFrame(QSize s, float opacity) : ParupaintFrame()
{
	SetOpacity(opacity);
	New(s);
}
void ParupaintFrame::LoadFromData(const QByteArray& ba)
{
	Frame.loadFromData(ba);	
}

void ParupaintFrame::New(QSize s)
{
	Frame = QImage(s, QImage::Format_ARGB32);
	ClearColor(QColor(0, 0, 0, 0));
}
void ParupaintFrame::Resize(QSize s)
{
	Frame = Frame.copy(QRect(0, 0, s.width(), s.height()));
}
QImage ParupaintFrame::GetImage() const
{
	return Frame;
}

void ParupaintFrame::ClearColor(QColor col)
{
	Frame.fill(col);
}

void ParupaintFrame::DrawStroke(ParupaintStroke * s)
{
	QPointF ppos;
	foreach(auto i, s->GetStrokes()){
		if(ppos.isNull()) ppos = i->GetPosition();

		const QPen pen = i->ToPen();
		const QPointF pos = i->GetPosition();

		QPainter painter(&Frame);
		painter.setPen(pen);
		painter.drawLine(ppos, pos);
		painter.end();

		ppos = pos;
	}
}

void ParupaintFrame::DrawStep(float x, float y, float x2, float y2, float width, QColor color)
{
	QPainter painter(&Frame);

	QPen pen(color);
	pen.setWidthF(width);
	painter.setPen(pen);

	painter.drawLine(x, y, x2, y2);
	painter.end();
}


void ParupaintFrame::SetOpacity(float o)
{
	Opacity = o;
}

void ParupaintFrame::SetExtended(bool e)
{
	Extended = e;
}

float ParupaintFrame::GetOpacity() const
{
	return Opacity;
}

bool ParupaintFrame::IsExtended()
{
	return Extended;
}
