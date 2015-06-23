#ifndef PARUPAINTBRUSHGLASS_H
#define PARUPAINTBRUSHGLASS_H

#include <QList>
#include "parupaintBrush.h"

class ParupaintBrushGlass
{
	private:
	QList<ParupaintBrush> brushes;
	ParupaintBrush * brushptr;
	int currentBrush;

	public:
	ParupaintBrushGlass();

	ParupaintBrush * GetCurrentBrush() ;
	int GetCurrentBrushNum() const;
	void ToggleBrush(int, int);
	void SetBrush(int);
};

#endif
