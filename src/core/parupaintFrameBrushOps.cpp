#include "parupaintFrameBrushOps.h"

#include "parupaintFrame.h"
#include "parupaintLayer.h"
#include "parupaintPanvas.h"
#include "parupaintBrush.h"

#include <QtMath>
#include <QDebug>
#include <QBitmap>

QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QPointF & pos, const QPointF & old_pos, const qreal s1)
{
	return ParupaintFrameBrushOps::stroke(panvas, brush, QLineF(old_pos, pos), s1);
}
QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QPointF & pos, const qreal s1)
{
	return ParupaintFrameBrushOps::stroke(panvas, brush, QLineF(pos, pos), s1);
}
QRect ParupaintFrameBrushOps::stroke(ParupaintPanvas * panvas, ParupaintBrush * brush, const QLineF & line, const qreal s1)
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
	if(brush->tool() == ParupaintBrushTool::BrushToolOpacityDrawing){
		size = brush->size();
	}

	QRect urect(op.toPoint() + QPoint(-size/2, -size/2), QSize(size, size));
	urect |= QRect(np.toPoint() + QPoint(-size/2, -size/2), QSize(size, size));

	QPen pen;
	QBrush pen_brush(color);
	pen.setCapStyle(Qt::RoundCap);
	pen.setWidthF(size);
	pen.setMiterLimit(size);

	if(brush->tool() == ParupaintBrushTool::BrushToolNone){
		pen.setMiterLimit((s1 > -1) ? s1 : size);
	}
	if(brush->pattern() != ParupaintBrushPattern::BrushPatternNone){
		pen_brush.setTextureImage(brush->patternImage());
	}
	pen.setBrush(pen_brush);

	switch(brush->tool()){
		case ParupaintBrushTool::BrushToolFloodFill: {
			urect = frame->drawFill(np, color, pen_brush);
			break;
		}
		// intentional fallthrough
		case ParupaintBrushTool::BrushToolOpacityDrawing: {

			color.setAlpha(brush->pressure() * color.alpha());
			if(color.alpha() == 0) color.setAlpha(1);

			pen_brush.setColor(color);
			pen.setBrush(pen_brush);
			pen.setColor(color);
		}
		default: {
			urect = frame->drawLine(QLineF(op, np), pen);
		}
	}
	return urect.adjusted(-1, -1, 1, 1);
}
