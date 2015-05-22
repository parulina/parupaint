
#include "parupaintCanvasBrushPool.h"
#include "parupaintCanvasBrush.h"

ParupaintCanvasBrushPool::ParupaintCanvasBrushPool()
{
	
}

void ParupaintCanvasBrushPool::AddCursor(QString str, ParupaintCanvasBrush * c)
{
	Cursors.insert(str, c);
}

void ParupaintCanvasBrushPool::RemoveCursor(QString str)
{
	Cursors.remove(str);
}
void ParupaintCanvasBrushPool::RemoveCursor(ParupaintCanvasBrush * c)
{
	foreach(auto i, Cursors.keys(c)){
		Cursors.remove(i);
	}
}

ParupaintCanvasBrush * ParupaintCanvasBrushPool::GetCursor(ParupaintBrush * brush)
{
	foreach(auto i, Cursors){
		// todo - can i compare canvasbrush to brush?
		if(i == brush){
			return i;
		}
	}
	return nullptr;
}

ParupaintCanvasBrush * ParupaintCanvasBrushPool::GetCursor(QString str)
{
	auto p = Cursors.find(str);
	if(p == Cursors.end()) return nullptr;
	else return (*p);
}

void ParupaintCanvasBrushPool::ClearCursors()
{
	Cursors.clear();
}
