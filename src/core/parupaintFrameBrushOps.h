#ifndef PARUPAINTFRAMEBRUSHOPS_H
#define PARUPAINTFRAMEBRUSHOPS_H

class ParupaintBrush;
class ParupaintFrame;

#include <QRect>

class ParupaintFrameBrushOps {
	public:
	static QRect stroke(float , float , float , float , ParupaintBrush * , ParupaintFrame *);
};

#endif
