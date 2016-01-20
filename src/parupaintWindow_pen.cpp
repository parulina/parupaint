#include "parupaintWindow.h"

#include <QDebug>

#include "parupaintCanvasScene.h"
#include "parupaintVisualCanvas.h" // RedrawCache
#include "parupaintVisualCursor.h" // line draw
#include "core/parupaintBrushGlass.h"
#include "core/parupaintFrameBrushOps.h" // Draw locally
#include "net/parupaintClientInstance.h"

#include <QtMath>

void ParupaintWindow::OnPenPress(const penInfo & info)
{
	ParupaintBrush * current_brush = brushes->brush();
	current_brush->setPosition(info.pos);
	
	current_brush->setPressure(info.pressure);

	if(info.buttons == Qt::LeftButton &&
	   canvas_state == noCanvasState){

		current_brush->setDrawing(true);
		if(client->connected() &&
		   (client->readOnly() || !client->isJoined()))
			current_brush->setDrawing(false);

		if(current_brush->drawing())
			canvas_state = canvasDrawingState;
	}
	if(info.buttons & Qt::MiddleButton){
		canvas_state = canvasMovingState;
		origin_pen = info.gpos;
		origin_zoom = view->zoom();
	}
	if(info.buttons & Qt::RightButton){
		if(canvas_state == canvasMovingState){
		} else {
			brushes->toggleBrush(1);
		}
	}

	if(current_brush->drawing()){
		if(current_brush->tool() == ParupaintBrushToolTypes::BrushToolLine){
			current_brush->setPosition(info.pos);
		} else {
			QRect r = ParupaintFrameBrushOps::stroke(scene->canvas(), current_brush, current_brush->pixelPosition());
			scene->canvas()->redraw(r);
		}
	}

	scene->updateMainCursor(brushes->brush());
	client->doBrushUpdate(current_brush);

	if(current_brush->tool() == ParupaintBrushToolTypes::BrushToolFloodFill){
		current_brush->setDrawing(false);
	}
}

void ParupaintWindow::OnPenMove(const penInfo& info)
{
	ParupaintBrush * current_brush = brushes->brush();
	current_pen = info.gpos;

	if(canvas_state == canvasMovingState){

		if((info.modifiers & Qt::ControlModifier) ||
		   (info.buttons & Qt::RightButton)){
			qreal dy = (origin_pen - info.gpos).y()/100 + origin_zoom;
			view->setZoom(dy);
		} else {
			view->moveView((info.old_gpos - info.gpos));
		}
		return;
	}
	if(canvas_state == canvasBrushZoomingState){
		qreal dy = (info.old_pos - info.pos).y();
		current_brush->setSize(current_brush->size() + dy);
		scene->updateMainCursor(current_brush);
		return;
	}

	if((current_brush->tool() == BrushToolLine && current_brush->drawing()) ||
	   (current_brush->tool() == BrushToolFloodFill && (info.modifiers & Qt::ShiftModifier))){
		scene->mainCursor()->setPosition(info.pos);
		if(current_brush->tool() == BrushToolLine){
			QLine line(current_brush->pixelPosition(), QPoint(qFloor(info.pos.x()), qFloor(info.pos.y())));
			this->scene->canvas()->setPreviewLine(line, current_brush->size(), current_brush->color());
		}
		return;
	}

	current_brush->setPosition(info.pos);
	current_brush->setPressure(info.pressure);

	if(scene->canvas()->hasPastePreview()){
		scene->canvas()->setPastePreviewPosition(scene->canvas()->matrix().map(info.pos));
	}

	bool send_update = false;
	if(current_brush->drawing()){
		send_update = true;
		QRect r = ParupaintFrameBrushOps::stroke(scene->canvas(), current_brush, info.pos, info.old_pos);
		scene->canvas()->redraw(r);
	} else {
		QLineF line(info.old_pos, info.pos);
		if(line.length() < 3) {
			send_update = true;
		}
	}
	scene->updateMainCursor(current_brush);

	if(send_update){
		client->doBrushUpdate(current_brush);
	}
}

void ParupaintWindow::OnPenRelease(const penInfo& info)
{
	ParupaintBrush * current_brush = brushes->brush();
	if(current_brush->tool() == ParupaintBrushToolTypes::BrushToolLine && current_brush->drawing()){

		current_brush->setPressure(1.0);
		QRect r = ParupaintFrameBrushOps::stroke(scene->canvas(), current_brush, info.pos, current_brush->position());
		scene->canvas()->redraw(r);

		this->scene->canvas()->setPreviewLine();

		bool signals_blocked = current_brush->blockSignals(true);
			current_brush->setPosition(info.pos);
		current_brush->blockSignals(signals_blocked);
		client->doBrushUpdate(current_brush);
	}
	current_brush->setPressure(0.0);
	current_brush->setDrawing(false);

	client->doBrushUpdate(current_brush);
	scene->updateMainCursor(current_brush);

	canvas_state = noCanvasState;
}
void ParupaintWindow::OnPenPointer(const penInfo& info)
{
	if(info.pointer == QTabletEvent::Eraser ||
	   info.pointer == QTabletEvent::Pen) {

		if(info.pointer == QTabletEvent::Eraser) {
			if(brushes->brushNum() != 1){
				brushes->toggleBrush(1);
				tablet_pen_switch = true;
			}
		} else if(info.pointer == QTabletEvent::Pen) {
			if(brushes->brushNum() == 1 && tablet_pen_switch){
				brushes->toggleBrush(1);
				tablet_pen_switch = false;
			}
		}
		scene->updateMainCursor(brushes->brush());
	}
}

void ParupaintWindow::OnPenScroll(QWheelEvent * event)
{
	float actual_zoom = (event->angleDelta().y() / 120.0) > 0 ? 1 : -1;
	if(event->modifiers() & Qt::ControlModifier || canvas_state == canvasMovingState){
		view->addZoom(actual_zoom * 0.2);
		view->showToast(QString("%1%").arg(view->zoom() * 100), 1000);
	} else {
		ParupaintBrush * current_brush = brushes->brush();
		current_brush->setSize(current_brush->size() + (actual_zoom * 4));
		scene->updateMainCursor(current_brush);
	}
}
