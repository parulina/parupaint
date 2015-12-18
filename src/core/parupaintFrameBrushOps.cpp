
#include "parupaintFrameBrushOps.h"
#include "parupaintFrame.h"
#include "parupaintLayer.h"
#include "parupaintPanvas.h"
#include "parupaintBrush.h"

#include <QtMath>
#include <QDebug>

QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QPointF & pos, const QPointF & old_pos)
{
	return ParupaintFrameBrushOps::stroke(panvas, brush, QLineF(old_pos, pos));
}
QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QPointF & pos)
{
	return ParupaintFrameBrushOps::stroke(panvas, brush, QLineF(pos, pos));
}
QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QLineF & line)
{
	ParupaintLayer * layer = panvas->layerAt(brush->layer());
	if(!layer) return QRect();

	ParupaintFrame * frame = layer->frameAt(brush->frame());
	if(!frame) return QRect();

	qreal size = brush->pressureSize();
	QColor color = brush->color();

	QPointF op = line.p1(), np = line.p2();
	// do not move this block
	if(size <= 1){
		size = 1;
		// pixel brush? pixel position.
		op = QPointF(qFloor(op.x()), qFloor(op.y()));
		np = QPointF(qFloor(np.x()), qFloor(np.y()));
	}

	QRect urect(op.toPoint() + QPoint(-size/2, -size/2), QSize(size, size));
	urect |= QRect(np.toPoint() + QPoint(-size/2, -size/2), QSize(size, size));

	QPen pen(color);
	pen.setCapStyle(Qt::RoundCap);
	pen.setWidthF(size);

	switch(brush->tool()){
		case ParupaintBrushToolTypes::BrushToolDotShadingPattern: {
			pen.setBrush(QBrush(color, Qt::Dense5Pattern)); break;
		}
		case ParupaintBrushToolTypes::BrushToolDotHighlightPattern: {
			pen.setBrush(QBrush(color, Qt::Dense6Pattern)); break;
		}
		case ParupaintBrushToolTypes::BrushToolCrossPattern: {
			pen.setBrush(QBrush(color, Qt::DiagCrossPattern)); break;
		}
	}
	switch(brush->tool()){
		case ParupaintBrushToolTypes::BrushToolFloodFill: {
			urect = frame->drawFill(np, color);
			break;
		}
		// intentional fallthrough
		case ParupaintBrushToolTypes::BrushToolOpacityDrawing: {
			double pressure = brush->pressure();
			if(pressure < 0.01) pressure = 0.01;
			color.setAlphaF(pressure * color.alphaF());
			pen.setColor(color);
		}
		default: {
			frame->drawLine(QLineF(op, np), pen);
		}
	}
	return urect.adjusted(-1, -1, 1, 1);
}
