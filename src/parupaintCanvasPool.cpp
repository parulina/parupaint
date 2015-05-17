
// Other users as well as the actual canvas.
//

#include <QDebug>

#include "parupaintCanvasPool.h"
#include "parupaintCanvasBrush.h"
#include "parupaintCanvasObject.h"


ParupaintCanvasPool::ParupaintCanvasPool(QObject * parent) : QGraphicsScene(parent)
{
	//Canvas = new ParupaintPanvas();
	Canvas = new ParupaintCanvasObject();
	Canvas->New(QSize(500, 500), 1, 1);
	addItem(Canvas);

	//setItemIndexMethod(NoIndex);
	
	setBackgroundBrush(QColor(255, 0, 0));
	ClearCursors();
}

void ParupaintCanvasPool::ClearCursors()
{
	foreach(auto *i, Cursors){
		delete i;
	}
	Cursors.clear();
}
