
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
void ParupaintStroke::SetLayerFrame(_lint l, _fint f)
{
	layer = l;
	frame = f;
}

void ParupaintStroke::SetBrush(ParupaintBrush * b)
{
	brush = b;
}
ParupaintBrush * ParupaintStroke::GetBrush() const
{
	return brush;
}
_lint ParupaintStroke::GetLayer() const
{
	return layer;
}
_fint ParupaintStroke::GetFrame() const
{
	return frame;
}

void ParupaintStroke::Clear()
{
	foreach(auto *i, strokes){
		delete i;
	}
	strokes.clear();
}
