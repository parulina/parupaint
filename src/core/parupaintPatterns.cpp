#include "parupaintPatterns.h"

#include <QDebug>

// Lifted from:
// src/gui/painting/qbrush.cpp
static uchar patterns[][8] = {
	{0xaa, 0x44, 0xaa, 0x11, 0xaa, 0x44, 0xaa, 0x11}, // Dense5Pattern
	{0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44}, // Dense6Pattern (modified to interweave Dense5Pattern)
	{0x81, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x81}, // DiagCrossPattern
	{0xFF, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80}  // Custom checker pattern
};

// QBitmap (inherits QPixmap) does not work on non-gui builds.
// Qt segfaults (???) if you attempt to do it anyways...
// so we end up using QImage instead as a brush texture.
QImage parupaintPattern(int pattern, const QColor & col)
{
	int num = sizeof(patterns)/sizeof(patterns[0]);
	if(pattern >= num) return QImage();

	QImage img(QSize(8, 8), QImage::Format_MonoLSB);
	img.setColor(0, QColor(Qt::transparent).rgba());
	img.setColor(1, QColor(Qt::white).rgba());
	if(col.alpha() != 0) img.setColor(1, col.rgba());

	for(int y = 0; y < 8; ++y){
		memcpy(img.scanLine(y), patterns[pattern] + y, 1);
	}
	return img;
}

