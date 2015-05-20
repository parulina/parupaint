#ifndef PARUPAINTBRUSHSTROKES_H
#define PARUPAINTBRUSHSTROKES_H

class ParupaintBrush;
class ParupaintStroke;

#include <QMultiMap>

class ParupaintBrushStrokes
{
	private:
	QMultiMap<ParupaintBrush *, ParupaintStroke *> strokes;

	public:
	ParupaintBrushStrokes();
	ParupaintBrush * GetBrushFromStroke(ParupaintStroke*);

	ParupaintStroke* NewBrushStroke(ParupaintBrush * brush);
	void EndBrushStroke(ParupaintBrush * brush);
	int GetNumBrushStrokes(ParupaintBrush * brush);
	int GetTotalStrokes();

	void Clear();
};

#endif
