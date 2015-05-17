
// Other users as well as the actual canvas.
//

#include <QDebug>

#include "parupaintCanvasPool.h"
#include "parupaintCanvasBrush.h"
#include "parupaintCanvasObject.h"

#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"

#include <QDebug>

ParupaintCanvasPool::ParupaintCanvasPool(QObject * parent) : QGraphicsScene(parent)
{
	Canvas = new ParupaintCanvasObject();
	Canvas->New(QSize(500, 500), 1, 1);
	Canvas->GetLayer(0)->GetFrame(0)->DrawStep(0, 0, 500, 500, 2, QColor(0, 0, 0));
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
