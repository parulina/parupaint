#ifndef PARUPAINTSTROKE_H
#define PARUPAINTSTROKE_H

class ParupaintBrush;
class ParupaintStrokeStep;

#include <QList>

class ParupaintStroke
{
	protected:
	ParupaintBrush * brush;
	QList<ParupaintStrokeStep *> strokes;
	// other attributes? such as ispressure, isremote, etc

	public:
	ParupaintStroke();

	void AddStroke(ParupaintStrokeStep *);
	void SetBrush(ParupaintBrush * brush);
	// remove everything and delete
	void Clear();
};

#endif
