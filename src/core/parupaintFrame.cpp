#include "parupaintFrame.h"

#include <QDebug>
#include <QtMath>
// draw* funcs
#include <QPainter>
#include <QPen>
#include <QRgb>
#include <QBitmap>

#include "parupaintLayer.h" // parentLayer
#include "parupaintSnippets.h"

ParupaintFrame::ParupaintFrame(QObject * parent) : QObject(parent)
{
}
ParupaintFrame::ParupaintFrame(const QSize & size, QObject * parent, qreal op) : ParupaintFrame(parent)
{
	frame = QImage(size, QImage::Format_ARGB32);
	this->clear(Qt::transparent);
	this->setOpacity(op);
}
ParupaintFrame::ParupaintFrame(const QImage & image, QObject * parent, qreal op) : ParupaintFrame(parent)
{
	frame = image;
	this->setOpacity(op);
}

ParupaintLayer * ParupaintFrame::parentLayer()
{
	return qobject_cast<ParupaintLayer*>(this->parent());
}

void ParupaintFrame::resize(const QSize & size)
{
	frame = frame.copy(QRect(QPoint(0, 0), size));
	emit onResize();
}
void ParupaintFrame::clear(const QColor color)
{
	frame.fill(color);
	emit onChange(frame.rect());
}
void ParupaintFrame::replaceImage(const QImage & img)
{
	frame = img;
	emit onChange(frame.rect());
}
void ParupaintFrame::setOpacity(qreal op)
{
	emit onOpacityChange();
	this->frame_opacity = op;
}

qreal ParupaintFrame::opacity()
{
	return this->frame_opacity;
}
const QImage & ParupaintFrame::image()
{
	return this->frame;
}
QImage ParupaintFrame::renderedImage()
{
	QImage img(this->frame.size(), this->frame.format());
	QPainter painter(&img);
	painter.setOpacity(this->opacity());

	img.fill(0);
	painter.drawImage(QPointF(0, 0), this->frame);
	painter.end();
	
	return img;
}

QRect ParupaintFrame::drawLine(const QLineF & line, const QColor color, const qreal s)
{
	return this->drawLine(line, color, s, s);
}
QRect ParupaintFrame::drawLine(const QLineF & line, const QColor color, const qreal s1, const qreal s2)
{
	QPen pen(color);
	pen.setWidthF(s1);
	pen.setMiterLimit(s2);
	pen.setCapStyle(Qt::RoundCap);
	return this->drawLine(line, pen);
}
QRect ParupaintFrame::drawLine(const QLineF & line, QPen pen)
{
	qreal ps = pen.widthF();
	QLine pixel_line(QPoint(qFloor(line.x1()), qFloor(line.y1())), QPoint(qFloor(line.x2()), qFloor(line.y2())));

	QRect changed_rect = QRect(line.p1().toPoint(), line.p2().toPoint()).adjusted(-ps, -ps, ps, ps);

	QPainter painter(&frame);

	QBrush new_brush = pen.brush();

	if(pen.color().alpha() == 0){
		painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
		new_brush.setColor(Qt::white);
	}

	if(line.length() == 0){
		painter.setPen(Qt::NoPen);
		painter.setBrush(new_brush);
		if(pen.width() == 1){
			painter.drawPoint(pixel_line.p2());
		} else {
			painter.drawEllipse(QRectF(line.p2() + QPointF(-ps/2, -ps/2), QSizeF(ps, ps)));
		}

	} else {
		qreal s1 = pen.widthF(), s2 = pen.miterLimit();

		pen.setBrush(new_brush);
		painter.setPen(pen);
		painter.setBrush(new_brush);

		if(s1 >= 1 && s2 >= 1 && (s1 > s2 ? s1-s2 : s2-s1) > 0.001){

			// i switched from trying to figuring out the tangent to simple angle calculation
			// if you would like to mess around with tangents and acos and stuff, be my guest
			const qreal d = line.length(),
			      r = qAtan2(line.y2() - line.y1(), line.x2() - line.x1()),
			      rr = r - M_PI/2;

			QVector<QPointF> points;
			if(s2 != 1){
				const qreal s = s2/2.0;
				points << line.p1() + QPointF(s * qCos(rr), s * qSin(rr));
				points << line.p1() - QPointF(s * qCos(rr), s * qSin(rr));
			} else points << line.p1();

			if(s1 != 1){
				const qreal s = s1/2.0;
				points << line.p2() - QPointF(s * qCos(rr), s * qSin(rr));
				points << line.p2() + QPointF(s * qCos(rr), s * qSin(rr));
			} else points << line.p2();

			QPainterPath path;

			path.addPolygon(QPolygonF(points));
			path.closeSubpath();

			if(s2 != 1){
				path.moveTo(line.p1());
				path.arcTo(QRectF(line.p1() - QPointF(s2/2, s2/2), QSizeF(s2, s2)), line.angle()+90, 180);
				path.closeSubpath();
			}
			/*
			// unsure if this is needed
			if(s1 != 1){
				path.moveTo(line.p2());
				path.arcTo(QRectF(line.p2() - QPointF(s1/2, s1/2), QSizeF(s1, s1)), line.angle()-90, 180);
				path.closeSubpath();
			}
			*/
			painter.fillPath(path, new_brush);

			changed_rect = path.boundingRect().toRect();

		} else {
			painter.drawLine(pixel_line);
			changed_rect = changed_rect.united(QRect(line.p1().toPoint() - QPoint(s2/2, s2/2), QSize(s2, s2)));
			changed_rect = changed_rect.united(QRect(line.p2().toPoint() - QPoint(s1/2, s1/2), QSize(s1, s1)));
		}
	}
	painter.end();

	emit onChange(changed_rect);
	return changed_rect;
}
QRect ParupaintFrame::drawImage(const QPointF & pos, const QImage & image)
{
	QPainter painter(&frame);
	QRectF target(pos, image.size()),
	       source(QPointF(0, 0), image.size());
	painter.drawImage(target, image, source);
	painter.end();

	QRect changed_rect = target.toRect() & frame.rect();
	emit onChange(changed_rect);
	return changed_rect;
}

QRegion imageToRegion(const QImage & img)
{
	QImage image = img.convertToFormat(QImage::Format_MonoLSB);
	QRegion region;
	QRect xr;


	// From Qt's sources: src/gui/painting/qregion.cpp
	// As with the case with brush patterns, i'd like to avoid
	// including the gui module to keep the server filesize down.
	// QRegion can only take QBitmap, which is dependent on the
	// gui module, which is dumb because internally it just converts
	// it back to QImage.

#define AddSpan \
	{ \
		xr.setCoords(prev1, y, x-1, y); \
		region = region.united(xr); \
	}

	const uchar zero = 0;
	bool little = image.format() == QImage::Format_MonoLSB;

	int x,
	    y;
	for (y = 0; y < image.height(); ++y) {
		const uchar *line = image.constScanLine(y);
		int w = image.width();
		uchar all = zero;
		int prev1 = -1;
		for (x = 0; x < w;) {
			uchar byte = line[x / 8];
			if (x > w - 8 || byte!=all) {
				if (little) {
					for (int b = 8; b > 0 && x < w; --b) {
						if (!(byte & 0x01) == !all) {
							// More of the same
						} else {
							// A change.
							if (all!=zero) {
								AddSpan
									all = zero;
							} else {
								prev1 = x;
								all = ~zero;
							}
						}
						byte >>= 1;
						++x;
					}
				} else {
					for (int b = 8; b > 0 && x < w; --b) {
						if (!(byte & 0x80) == !all) {
							// More of the same
						} else {
							// A change.
							if (all != zero) {
								AddSpan
									all = zero;
							} else {
								prev1 = x;
								all = ~zero;
							}
						}
						byte <<= 1;
						++x;
					}
				}
			} else {
				x += 8;
			}
		}
		if (all != zero) {
			AddSpan
		}
	}
#undef AddSpan

	return region;
}

QRect ParupaintFrame::drawFill(const QPointF & pos, QColor to_color, const QBrush & brush)
{
	ParupaintFillHelper help(frame);
	QRect rect = help.fill(pos.x(), pos.y(), to_color.rgba());

	QPainter painter(&frame);
	painter.setClipRegion(imageToRegion(help.mask()));

	QBrush nb = brush;
	if(nb.color().alpha() == 0){
		nb.setColor(Qt::white);
		painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
	}
	painter.setBrush(nb);

	painter.fillRect(frame.rect(), nb);
	painter.end();

	emit onChange(rect);
	return rect;
}
