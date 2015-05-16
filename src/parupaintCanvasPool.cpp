
// Other users as well as the actual canvas.
//
#include "parupaintCanvasPool.h"


ParupaintCanvasPool::ParupaintCanvasPool(QObject * parent) : QGraphicsScene(parent)
{
	Canvas = new ParupaintCanvasPanvas();


	//setItemIndexMethod(NoIndex);
	
	foreach(auto *i, Cursors){
		delete i;
	}
	Cursors.clear();
}
