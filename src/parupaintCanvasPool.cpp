
// Other users as well as the actual canvas.
//

#include "parupaintCanvasPool.h"
#include "parupaintCanvasObject.h"
#include "parupaintCursor.h"
#include "parupaintBrush.h"

#include "panvas/parupaintPanvas.h"
#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"


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



void ParupaintCanvasPool::AddCursor(QString str, ParupaintCursor * c)
{
	ParupaintCursorPool::AddCursor(str, c);
	addItem(c);
}
void ParupaintCanvasPool::RemoveCursor(ParupaintCursor * c)
{
	ParupaintCursorPool::RemoveCursor(c);
	removeItem(c);
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




// client brush strokes
ParupaintCanvasStrokeObject * ParupaintCanvasPool::NewBrushStroke(ParupaintBrush * brush)
{
	ParupaintCanvasStrokeObject * stroke = new ParupaintCanvasStrokeObject(this->GetCanvas());
	brush->SetCurrentStroke(stroke);
	stroke->SetBrush(brush);
	
	strokes.insert(brush, stroke);
	addItem(stroke);
	return stroke;
}

void ParupaintCanvasPool::EndBrushStroke(ParupaintBrush * brush)
{
// 	brush->SetCurrentStroke(nullptr);
}

int ParupaintCanvasPool::GetNumBrushStrokes(ParupaintBrush * brush)
{
	return strokes.values(brush).length();
}
int ParupaintCanvasPool::GetTotalStrokes()
{
	int i = 0;
	for(auto i = strokes.begin(); i != strokes.end(); ++i){
		i += GetNumBrushStrokes(i.key());
	}
	return i;
}

void ParupaintCanvasPool::ClearStrokes()
{
	foreach(auto i, strokes){
		delete i;
	}
	strokes.clear();
	
}
