
#include "parupaintBrushStrokes.h"
#include "../parupaintBrush.h"
#include "parupaintStroke.h"


ParupaintBrushStrokes::ParupaintBrushStrokes()
{
	
}

ParupaintStroke* ParupaintBrushStrokes::NewBrushStroke(ParupaintBrush * brush)
{
	ParupaintStroke * stroke = new ParupaintStroke();
	brush->SetCurrentStroke(stroke);
	stroke->SetBrush(brush);
	if(!strokes.isEmpty()) stroke->SetPreviousStroke(strokes.last());

	strokes.insert(brush, stroke);
	// connect them together
	
	return stroke;
}

void ParupaintBrushStrokes::EndBrushStroke(ParupaintBrush * brush)
{
	brush->SetCurrentStroke(nullptr);
}

int ParupaintBrushStrokes::GetNumBrushStrokes(ParupaintBrush * brush)
{
	return strokes.values(brush).length();
}
int ParupaintBrushStrokes::GetTotalStrokes()
{
	int i = 0;
	for(auto i = strokes.begin(); i != strokes.end(); ++i){
		i += GetNumBrushStrokes(i.key());
	}
	return i;
}

void ParupaintBrushStrokes::Clear()
{
	foreach(auto i, strokes){
		delete i;
	}
	strokes.clear();
	
}
