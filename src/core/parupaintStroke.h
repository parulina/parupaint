#ifndef PARUPAINTSTROKE_H
#define PARUPAINTSTROKE_H

class ParupaintBrush;
class ParupaintStrokeStep;

#include "panvasTypedefs.h"
#include <QPixmap>
#include <QList>

class ParupaintStroke
{
	protected:
	ParupaintBrush * brush;
	ParupaintStroke * previousStroke;
	ParupaintStroke * nextStroke;
	QList<ParupaintStrokeStep *> strokes;

	// other attributes? such as ispressure, isremote, etc

	public:
	ParupaintStroke();

	virtual void AddStroke(ParupaintStrokeStep *);
	void SetBrush(ParupaintBrush * brush);

	QList<ParupaintStrokeStep*> GetStrokes() const;

	void SetPreviousStroke(ParupaintStroke *);
	ParupaintStroke* GetPreviousStroke();
	void SetNextStroke(ParupaintStroke *);
	ParupaintStroke* GetNextStroke();
	
	// NOTE! ! THIS DOESNT RETURN THE STROKE BRUSH USED, 
	// THIS RETURNS THE PARENT!!
	ParupaintBrush * GetBrush() const;
	// remove everything and delete
	void Clear();
};

#endif