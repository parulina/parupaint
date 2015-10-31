
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QRgb>

#include "parupaintFrame.h"
#include "parupaintStrokeStep.h"
#include "parupaintStroke.h"

#include "parupaintSnippets.h"

ParupaintFrame::~ParupaintFrame()
{

}

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

void ParupaintFrame::Replace(QImage img)
{
	Frame = img;
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

void ParupaintFrame::DrawStep(float x, float y, float x2, float y2, QPen & pen)
{
	QPainter painter(&Frame);
	painter.setPen(pen);
	if(pen.color().alpha() == 0) painter.setCompositionMode(QPainter::CompositionMode_Clear);
	painter.drawLine(x, y, x2, y2);
	painter.end();
}

void ParupaintFrame::DrawStep(float x, float y, float x2, float y2, float width, QColor color)
{
	QPen pen(color);
	pen.setWidthF(width < 1 ? 1 : width);
	pen.setCapStyle(Qt::RoundCap);
	this->DrawStep(x, y, x2, y2, pen);
}

QRect ParupaintFrame::Fill(int x, int y, QColor color)
{
	ParupaintFillHelper help(Frame);
	QRect rect = help.fill(x, y, color.rgba());
	Frame = help.image();
	return rect;
}

void ParupaintFrame::DrawImage(int x, int y, QImage img)
{
	QPainter painter(&Frame);
	QRect target(x, y, Frame.width(), Frame.height());
	QRect source(0, 0, Frame.width(), Frame.height());
	painter.drawImage(target, img, source);
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

bool ParupaintFrame::IsExtended() const
{
	return Extended;
}
