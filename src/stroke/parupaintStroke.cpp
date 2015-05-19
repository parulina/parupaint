
#include "parupaintStroke.h"
#include "parupaintStrokeStep.h"

ParupaintStroke::ParupaintStroke()
{

}

// !! step gets ownership. do not delete it
void ParupaintStroke::AddStroke(ParupaintStrokeStep * step)
{
	strokes.append(step);	
}

void ParupaintStroke::SetBrush(ParupaintBrush * brush)
{

}

void ParupaintStroke::Clear()
{
	foreach(auto *i, strokes){
		delete i;
	}
	strokes.clear();
}
