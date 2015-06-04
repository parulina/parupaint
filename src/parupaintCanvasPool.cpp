
// Other users as well as the actual canvas.
//

#include <QPainter>

#include "parupaintCanvasPool.h"
#include "parupaintCanvasObject.h"
#include "parupaintCanvasStrokeObject.h"
#include "parupaintCanvasBrush.h"
#include "core/parupaintBrush.h"
#include "core/parupaintStrokeStep.h"

#include "core/parupaintPanvas.h"
#include "core/parupaintLayer.h"
#include "core/parupaintFrame.h"

ParupaintCanvasPool::~ParupaintCanvasPool()
{

}

ParupaintCanvasPool::ParupaintCanvasPool(QObject * parent) : QGraphicsScene(parent)
{
	Canvas = new ParupaintCanvasObject();
	// Need to be here to trigger resize
	connect(Canvas, SIGNAL(ResizeSignal(QSize, QSize)), this, SLOT(OnCanvasResize(QSize, QSize)));


	Canvas->New(QSize(500, 500), 1, 1);
	addItem(Canvas);

	//setItemIndexMethod(NoIndex);
	connect(Canvas, SIGNAL(CurrentSignal(int, int)), this, SLOT(CurrentChange(int,int)));
	
	setBackgroundBrush(QColor(255, 0, 0));
	ClearCursors();
}
void ParupaintCanvasPool::TriggerViewUpdate()
{
	emit UpdateView();
}

ParupaintCanvasObject * ParupaintCanvasPool::GetCanvas()
{
	return Canvas;
}



void ParupaintCanvasPool::AddCursor(QString str, ParupaintCanvasBrush * c)
{
	ParupaintCanvasBrushPool::AddCursor(str, c);
	addItem(c);
}
void ParupaintCanvasPool::RemoveCursor(ParupaintCanvasBrush * c)
{
	ParupaintCanvasBrushPool::RemoveCursor(c);
	removeItem(c);
}



void ParupaintCanvasPool::OnCanvasResize(QSize old_size, QSize new_size)
{
	QSize size = new_size - old_size;
	QRectF bounds = Canvas->boundingRect();
	const float padding = 200;
	
	// Add the padding
	setSceneRect(bounds.adjusted(-padding, -padding, padding, padding));
	
	foreach(auto i, strokes){
		i->SetRegionLimit(bounds);
	}

	emit UpdateCanvas();
	emit UpdateView();
}




ParupaintCanvasStrokeObject * ParupaintCanvasPool::NewBrushStroke(ParupaintBrush * brush)
{
	if(!strokes.values(brush).isEmpty()){
		while(brush->GetLastStroke() != strokes.values(brush).first()){
			auto * stroke = strokes.values(brush).first();
			delete stroke;
			strokes.remove(brush, stroke);

			if(strokes.values(brush).isEmpty()) break;
			// todo - what if it gets empty?
			// what happens with last stroke? should we reset it?
		}
	}
	if(strokes.values(brush).isEmpty()) {
		// add a dummy first thing
		// otherwise on undo it'll set 0x00 and forget
		// about the next stroke
		strokes.insert(brush, new ParupaintCanvasStrokeObject());
	}

	ParupaintCanvasStrokeObject * stroke = new ParupaintCanvasStrokeObject(Canvas->boundingRect());
	brush->SetCurrentStroke(stroke);
	stroke->SetBrush(brush);


	stroke->SetPreviousStroke(nullptr); // grumble grumble..

	// put next/prev pointers
	auto * first = strokes.values(brush).first();
	first->SetNextStroke(stroke);
	stroke->SetPreviousStroke(first);

	strokes.insert(brush, stroke);
	// first = new, last = 0
	// begin = new, end = 0

	addItem(stroke);
	return stroke;
}

void ParupaintCanvasPool::EndBrushStroke(ParupaintBrush * brush)
{
	if(brush->GetCurrentStroke()){
		brush->SetLastStroke(brush->GetCurrentStroke());
	}
	brush->SetCurrentStroke(nullptr);
}

void ParupaintCanvasPool::UpdateBrushStrokes(ParupaintBrush * brush)
{
	bool passed = false;
	for(auto i = strokes.begin(); i != strokes.end(); ++i){
		auto *scene_item = *i;
		
		if(brush->GetLastStroke() == dynamic_cast<ParupaintStroke*>(scene_item)) passed = true;
		
		if(scene_item){
			if(!passed && scene_item->isVisible()) {
				scene_item->hide();

			} else if(passed && !scene_item->isVisible()) {
				scene_item->show();
			}
		}
	}
}

void ParupaintCanvasPool::UndoBrushStroke(ParupaintBrush * brush)
{
	auto * stroke = brush->GetLastStroke();
	if(stroke && stroke->GetPreviousStroke()) {
		brush->SetLastStroke(stroke->GetPreviousStroke());
		UpdateBrushStrokes(brush);
	}
}

void ParupaintCanvasPool::RedoBrushStroke(ParupaintBrush * brush)
{
	auto * stroke = brush->GetLastStroke();
	if(stroke && stroke->GetNextStroke()) {
		brush->SetLastStroke(stroke->GetNextStroke());
		UpdateBrushStrokes(brush);
	}
}
void ParupaintCanvasPool::SquashBrushStrokes(ParupaintBrush * brush)
{
	if(!strokes.values(brush).isEmpty()){
		while(brush->GetLastStroke() != strokes.values(brush).first()){
			auto * stroke = strokes.values(brush).first();
			delete stroke;
			strokes.remove(brush, stroke);

			if(strokes.values(brush).isEmpty()) break;
		}
	}

	auto val = strokes.values(brush);
	for(int i = 0; i < val.length(); i++){
		auto s = val.at(i);

		QPointF pos1;
		foreach(auto i, s->GetStrokes()){
			if(pos1.isNull()) pos1 = i->GetPosition();
			QPointF pos2 = i->GetPosition();

			auto * layer = this->GetCanvas()->GetLayer(i->GetLayer());
			if(!layer) continue;
			auto * frame = layer->GetFrame(i->GetFrame());
			if(!frame) continue;
			
			frame->DrawStep(pos1.x(), pos1.y(), pos2.x(), pos2.y(), i->GetWidth(), i->GetColor());
			// putting the pixmap on the frame instead
			// redrawing the lines makes the lines different?!?!
			// + useless work
			/*
			QImage tmp_img = frame->GetImage();
			QPainter painter(&tmp_img);
			painter.drawPixmap(frame->GetImage().rect(), s->pixmap());
			frame->Replace(tmp_img);
			*/
			pos1 = pos2;
		}
	}
	this->ClearBrushStrokes(brush);
}
void ParupaintCanvasPool::ClearBrushStrokes(ParupaintBrush * brush)
{
	foreach(auto i, strokes.values(brush)){
		delete i;
	}
	strokes.remove(brush);
	brush->SetLastStroke(nullptr);
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

void ParupaintCanvasPool::CurrentChange(int l, int f)
{
	foreach(auto i, strokes){
		i->hide();
		// todo - this shows all strokesteps, even though they might be
		// in other l/f's. how to resolve?
		// might make a func to loop through them and redraw only the
		// ones that match l/f. probably the best choice
		//
		// over 2 because 0 is a null thing
		

		if(i->GetStrokes().length() < 2) continue;
		if(i->GetStrokes().at(1)-> GetLayer() == l && i->GetStrokes().at(1)->GetFrame() == f){
			i->show();
		}
	}
	this->TriggerViewUpdate();
}
