#ifndef PARUPAINTCOMMONOPERATIONS_H
#define PARUPAINTCOMMONOPERATIONS_H

#include <QVariant>

class ParupaintPanvas;
class ParupaintBrush;

class ParupaintCommonOperations
{
	public:
	// canvas
	static bool CanvasAttributeOp(ParupaintPanvas * canvas, const QString & attr, const QVariant & val);
	static bool CanvasResizeOp(ParupaintPanvas * canvas, int w, int h, bool resize = true);
	static bool LayerFramePasteOp(ParupaintPanvas * canvas, int l, int f, int x, int y, const QImage & img);
	static bool LayerFrameFillOp(ParupaintPanvas * canvas, int l, int f, const QColor & color);
	static bool LayerFrameChangeOp(ParupaintPanvas * canvas, int l, int f, int lc, int fc, bool extend);
	static bool LayerFrameAttributeOp(ParupaintPanvas * canvas, int l, int f, const QString & attr, const QVariant & val);

	// brush
	static bool BrushOp(ParupaintBrush * brush, QLineF & line, const QVariantMap & data);
	static void AdjustBrush(ParupaintBrush * brush, ParupaintPanvas * canvas);
};

#endif
