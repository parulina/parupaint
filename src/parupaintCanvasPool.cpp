
// Other users as well as the actual canvas.
//

#include <QDebug>

#include "parupaintCanvasPool.h"
#include "parupaintCanvasBrush.h"
#include "parupaintCanvasObject.h"

#include "panvas/parupaintPanvas.h"
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
	connect(Canvas, SIGNAL(ResizeSignal(QSize, QSize)), this, SLOT(OnCanvasResize(QSize, QSize)));
	
	Canvas->Resize(QSize(200, 200));

	setBackgroundBrush(QColor(255, 0, 0));
	ClearCursors();
}

ParupaintCanvasObject * ParupaintCanvasPool::GetCanvas()
{
	return Canvas;
}

void ParupaintCanvasPool::ClearCursors()
{
	foreach(auto *i, Cursors){
		delete i;
	}
	Cursors.clear();
}


void ParupaintCanvasPool::OnCanvasResize(QSize old_size, QSize new_size)
{
	QSize size = new_size - old_size;
	QRectF bounds = Canvas->boundingRect();
	const float padding = 200;
	
	// Add the padding
	setSceneRect(bounds.adjusted(-padding, -padding, padding, padding));
	
	emit UpdateView();
}

