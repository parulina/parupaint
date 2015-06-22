
#include <QPainter>
#include <QPen>

#include "parupaintFrame.h"
#include "parupaintStrokeStep.h"
#include "parupaintStroke.h"

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

void p_fill(QImage & img, int x, int y, QRgb orig, QRgb to)
{
	if(orig == to) return;

	const QRect r = img.rect();
	if(!r.contains(x, y)) return;

	QRgb pp = img.pixel(x, y);
	if(pp != orig) return;

	QList<QPoint> plist = {QPoint(x, y)};

	while(!plist.isEmpty()){
		auto w = plist.front(), e = w;

		QRgb temp_color;
		temp_color = img.pixel(w);
		while(temp_color == orig){
			if(w.x() <= 0) break;
			w.setX(w.x()-1);
			temp_color = img.pixel(w);
		}
		temp_color = img.pixel(e);
		while(temp_color == orig){
			if(e.x() >= r.width()-1) break;
			e.setX(e.x()+1);
			temp_color = img.pixel(e);
		}

		for(int x = w.x(); x < e.x(); x++){
			int px = x;
			int py = plist.front().y();

			img.setPixel(px, py, to);
			if(py > 0 && img.pixel(px, py - 1) == orig) plist.append(QPoint(px, py-1));
			if(py < r.height()-1 && img.pixel(px, py + 1) == orig) plist.append(QPoint(px, py+1));
		}
		plist.pop_front();

	}
}

void ParupaintFrame::Fill(int x, int y, QColor color)
{
	p_fill(Frame, x, y, Frame.pixel(x, y), color.rgba());
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
