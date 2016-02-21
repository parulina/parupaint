#ifndef PARUPAINTFRAMEBRUSHOPS_H
#define PARUPAINTFRAMEBRUSHOPS_H

class ParupaintBrush;
class ParupaintFrame;
class ParupaintPanvas;

#include <QRect>
#include <QLineF>

class ParupaintFrameBrushOps {
	public:
	static QRect stroke(ParupaintPanvas *, ParupaintBrush *, const QPointF & pos, const QPointF & old_pos, const qreal s1 = -1);
	static QRect stroke(ParupaintPanvas *, ParupaintBrush *, const QPointF & pos, const qreal s1 = -1);
	static QRect stroke(ParupaintPanvas *, ParupaintBrush *, const QLineF & line, const qreal s1 = -1);
};

#endif
