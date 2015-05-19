#ifndef PARUPAINTSTROKE_H
#define PARUPAINTSTROKE_H

class ParupaintBrush;
class ParupaintStrokeStep;

#include "../panvas/panvasTypedefs.h"
#include <QPixmap>
#include <QList>

class ParupaintStroke
{
	protected:
	ParupaintBrush * brush;
	QList<ParupaintStrokeStep *> strokes;
	_lint layer;
	_fint frame;

	// other attributes? such as ispressure, isremote, etc

	public:
	ParupaintStroke();

	virtual void AddStroke(ParupaintStrokeStep *);
	void SetLayerFrame(_lint, _fint);
	void SetBrush(ParupaintBrush * brush);
	// NOTE! ! THIS DOESNT RETURN THE STROKE BRUSH USED, 
	// THIS RETURNS THE PARENT!!
	ParupaintBrush * GetBrush() const;
	_lint GetLayer() const;
	_fint GetFrame() const;
	// remove everything and delete
	void Clear();
};

#endif
