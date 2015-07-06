#ifndef PARUPAINTFRAMEBRUSHOPS_H
#define PARUPAINTFRAMEBRUSHOPS_H

class ParupaintBrush;
class ParupaintFrame;
class ParupaintPanvas;

#include <QRect>

class ParupaintFrameBrushOps {
	public:
	static QRect stroke(ParupaintPanvas *, float , float , float , float , ParupaintBrush *);
};

#endif
