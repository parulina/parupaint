#include "parupaintFrame.h"

#include <QDebug>
#include <QtMath>
// draw* funcs
#include <QPainter>
#include <QPen>
#include <QRgb>

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

		if(s1 >= 1 && s2 >= 1 && (s1 > s2 ? s1-s2 : s2-s1) > 0.1){
			const qreal d = line.length(),
				    r = qAtan2(line.y2() - line.y1(), line.x2() - line.x1()),
			            dr = (s1 > s2) ? s1 - s2 : s2 - s1;
			const qreal tan = qSqrt((d * d) + (dr * dr));

			const qreal p1 = qSqrt((tan * tan) + (s2 * s2)),
			            p2 = qSqrt((tan * tan) + (s1 * s1));

			const qreal theta1 = qAcos((s1*s1 + d*d - p1*p1) / (2 * s1 * d)) + r,
			            theta2 = qAcos((s2*s2 + d*d - p2*p2) / (2 * s2 * d)) + r;

			QPointF tf1((s2/2) * qCos(theta1), (s2/2) * qSin(theta1)),
				tf2((s1/2) * qCos(theta2), (s1/2) * qSin(theta2));

			const QPolygonF poly(QVector<QPointF>{
				line.p1() + tf1,
				line.p2() + tf2,
				line.p2() + QPointF(-(s1/2) * qCos(theta1), -(s1/2) * qSin(theta1)),
				line.p1() + QPointF(-(s2/2) * qCos(theta2), -(s2/2) * qSin(theta2))
			});
			QPainterPath path;
			path.addPolygon(poly);
			painter.fillPath(path, new_brush);

			painter.setPen(Qt::NoPen);
			painter.drawEllipse(line.p1(), s2/2, s2/2);
			painter.drawEllipse(line.p2(), s1/2, s1/2);

		} else {
			painter.drawLine(pixel_line);
		}
	}
	painter.end();

	ps = pen.width();
	QRect changed_rect = QRect(line.p1().toPoint(), line.p2().toPoint()).adjusted(-ps, -ps, ps, ps);
	emit onChange(changed_rect);
	return changed_rect;
}
QRect ParupaintFrame::drawImage(const QPointF & pos, const QImage & image)
{
	QPainter painter(&frame);
	QRectF target(pos, image.size()),
	       source(QPointF(0, 0), image.size());
	painter.drawImage(target, image, source);

	QRect changed_rect = target.toRect() & frame.rect();
	emit onChange(changed_rect);
	return changed_rect;
}
QRect ParupaintFrame::drawFill(const QPointF & pos, QColor to_color, QColor from_color)
{
	if(!from_color.isValid()){
		from_color = frame.pixel(pos.toPoint());
	}
	ParupaintFillHelper help(frame);
	QRect rect = help.fill(pos.x(), pos.y(), to_color.rgba());
	this->drawImage(QPointF(0, 0), help.mask());

	emit onChange(rect);
	return rect;
}
