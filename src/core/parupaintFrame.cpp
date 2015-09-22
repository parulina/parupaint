
#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QRgb>

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
	if(img.format() != QImage::Format_ARGB32) return;
	if(orig == to) return;

	const QRect r = img.rect();
	if(!r.contains(x, y)) return;

	QRgb pp = img.pixel(x, y);
	if(pp != orig) return;

	const int ww = r.size().width();
	const int hh = r.size().height();
	QList<QPoint> plist = {QPoint(x, y)};
	plist.reserve(ww*hh);

	uchar * img_bits = img.bits();
	uchar * img_data = new uchar[img.byteCount()];
	memcpy(img_data, img_bits, img.byteCount());

	// A B G R
	while(!plist.isEmpty()){

		const QPoint p = plist.front();
		const int ty = p.y();
		int tx = p.x(), tx2 = tx;

		while(tx > 0){
			tx--;
			if(*(QRgb*)(img_data + (4 * (tx + (ty * ww)))) != orig){
				tx++;
				break;
			}
		}
		while(tx2 < ww){
			tx2++;
			if(tx2 >= ww) break; // .pixel(.width) doesn't work for some reason
			if(*(QRgb*)(img_data + (4 * (tx2 + (ty * ww)))) != orig){
				break;
			}
		}

		for(int x = tx; x < tx2; x++){
			*(QRgb*)(img_data + (4 * (x + (ty * ww)))) = to;
			if(ty > 0 && *(QRgb*)(img_data + (4 * (x + ((ty - 1) * ww)))) == orig) { plist.append(QPoint(x, ty-1)); }
			if(ty < hh-1 && *(QRgb*)(img_data + (4 * (x + ((ty + 1) * ww)))) == orig) { plist.append(QPoint(x, ty+1)); }
		}
		plist.pop_front();

	}
	img = QImage(img_data, ww, hh, img.format());
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
