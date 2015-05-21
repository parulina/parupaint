
#include "parupaintStroke.h"
#include "parupaintStrokeStep.h"

ParupaintStroke::ParupaintStroke()
{
	SetPreviousStroke(nullptr);	
	SetNextStroke(nullptr);	
}

// !! step gets ownership. do not delete it
void ParupaintStroke::AddStroke(ParupaintStrokeStep * step)
{
	strokes.append(step);	
}

void ParupaintStroke::SetBrush(ParupaintBrush * b)
{
	brush = b;
}
ParupaintBrush * ParupaintStroke::GetBrush() const
{
	return brush;
}
void ParupaintStroke::Clear()
{
	foreach(auto *i, strokes){
		delete i;
	}
	strokes.clear();
}


void ParupaintStroke::SetNextStroke(ParupaintStroke *s)
{
	nextStroke = s;	
}
ParupaintStroke* ParupaintStroke::GetNextStroke()
{
	return nextStroke;
}

void ParupaintStroke::SetPreviousStroke(ParupaintStroke *s)
{
	previousStroke = s;	
}
ParupaintStroke* ParupaintStroke::GetPreviousStroke()
{
	return previousStroke;
}



QList<ParupaintStrokeStep*> ParupaintStroke::GetStrokes() const
{
	return strokes;
}
