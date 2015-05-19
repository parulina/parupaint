
#include "parupaintCursorPool.h"
#include "parupaintCursor.h"

ParupaintCursorPool::ParupaintCursorPool()
{
	
}

void ParupaintCursorPool::AddCursor(QString str, ParupaintCursor * c)
{
	Cursors.insert(str, c);
}

void ParupaintCursorPool::RemoveCursor(QString str)
{
	Cursors.remove(str);
}
void ParupaintCursorPool::RemoveCursor(ParupaintCursor * c)
{
	foreach(auto i, Cursors.keys(c)){
		Cursors.remove(i);
	}
}

ParupaintCursor * ParupaintCursorPool::GetCursor(ParupaintBrush * brush)
{
	foreach(auto i, Cursors){
		if(i->GetBrush() == brush){
			return i;
		}
	}
	return nullptr;
}

ParupaintCursor * ParupaintCursorPool::GetCursor(QString str)
{
	auto p = Cursors.find(str);
	if(p == Cursors.end()) return nullptr;
	else return (*p);
}

void ParupaintCursorPool::ClearCursors()
{
	Cursors.clear();
}
