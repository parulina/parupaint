
#include "parupaintCursorPool.h"


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
