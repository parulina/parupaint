
#include <QScrollBar>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QBitmap>
#include <QShortcut>
#include <QKeySequence>
#include <QObject>
#include <cmath>

#include <QDebug>
#include <QTimer>

#include "parupaintCanvasBrush.h"
#include "core/parupaintBrush.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"
#include "parupaintCanvasBrushPool.h"
#include "parupaintCanvasObject.h"

//QGraphicsView for canvas view


ParupaintCanvasView::ParupaintCanvasView(QWidget * parent) : QGraphicsView(parent), CurrentCanvas(nullptr),
	// Canvas stuff
	CanvasState(CANVAS_STATUS_IDLE), PenState(PEN_STATE_UP), Zoom(1.0), Drawing(false),
	// Brush stuff
	CurrentBrush(nullptr),
	// Button stuff
	DrawButton(Qt::LeftButton), MoveButton(Qt::MiddleButton), SwitchButton(Qt::RightButton)
{
	// mouse pointers and canvas itself
	//
	
	this->setFocusPolicy(Qt::WheelFocus);
	setBackgroundBrush(QColor(200, 200, 200));
	viewport()->setMouseTracking(true);
	
	SetZoom(Zoom);
}


void ParupaintCanvasView::UpdateCurrentBrush(ParupaintBrush * brush)
{
	if(CurrentBrush){
		*((ParupaintBrush*)CurrentBrush) = *brush;
	}
}

void ParupaintCanvasView::SetCurrentBrush(ParupaintBrush * brush)
{
	if(CurrentBrush){
		brush->SetPosition(CurrentBrush->GetPosition());
	}
	if(CurrentBrush && CurrentCanvas) {
		CurrentCanvas->RemoveCursor(CurrentBrush);
		delete CurrentBrush;
	}

	if(brush) {
		ParupaintCanvasBrush * cursor = new ParupaintCanvasBrush;
		viewport()->setCursor(Qt::BlankCursor);
		CurrentBrush = cursor;
		this->UpdateCurrentBrush(brush);
		CurrentBrush->SetPosition(brush->GetPosition());
		CurrentBrush->SetDrawing(brush->IsDrawing());
		// copy the options

		if(CurrentCanvas) CurrentCanvas->AddCursor(" ", cursor);
	} else {
		viewport()->setCursor(Qt::ArrowCursor);
		CurrentBrush = nullptr;
	}

} 

void ParupaintCanvasView::SetCanvas(ParupaintCanvasPool * canvas)
{
	CurrentCanvas = canvas;
	setScene(canvas);
	if(CurrentBrush) CurrentCanvas->AddCursor(" ", CurrentBrush);

	connect(canvas, SIGNAL(UpdateView()), this, SLOT(OnCanvasUpdate()));
}

float ParupaintCanvasView::GetZoom() const
{
	return Zoom;
}
void ParupaintCanvasView::SetZoom(float z)
{
	if(z < 0.2) z = 0.2;
	Zoom = z;
	
	QMatrix nm(1,0,0,1, matrix().dx(), matrix().dy());
	nm.scale(Zoom, Zoom);

	setMatrix(nm);
}
void ParupaintCanvasView::AddZoom(float z)
{
	SetZoom(Zoom + z);
}



// Events

void ParupaintCanvasView::OnPenDown(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure)
{
	if(CurrentBrush == nullptr) return;

	CurrentBrush->SetPosition(RealPosition(pos));
	CurrentBrush->SetPressure(pressure);

	if(buttons == DrawButton && !Drawing){
		Drawing = true;
		CurrentBrush->SetDrawing(Drawing);
		emit PenDrawStart(CurrentBrush);
		emit CursorChange(CurrentBrush);

	} else if(buttons == MoveButton && CanvasState != CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_MOVING;
	}
	viewport()->update();
}

void ParupaintCanvasView::OnPenUp(const QPointF &pos, Qt::MouseButtons buttons)
{
	if(CurrentBrush == nullptr) return;

	CurrentBrush->SetPosition(RealPosition(pos));
	CurrentBrush->SetPressure(0.0);
	if(Drawing){
		Drawing = false;
		CurrentBrush->SetDrawing(Drawing);
		emit PenDrawStop(CurrentBrush);
		emit CursorChange(CurrentBrush);
	}
	if(CanvasState == CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_IDLE;
	}

	viewport()->update();
}

void ParupaintCanvasView::OnPenMove(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure)
{
	if(CurrentBrush == nullptr) return;

	if(CanvasState == CANVAS_STATUS_MOVING){
		QScrollBar *ver = verticalScrollBar();
		QScrollBar *hor = horizontalScrollBar();
		auto dif = OldPosition.toPoint() - pos.toPoint();
		hor->setSliderPosition(hor->sliderPosition() + dif.x());
		ver->setSliderPosition(ver->sliderPosition() + dif.y());

	} else if(CanvasState == CANVAS_STATUS_ZOOMING || CanvasState == CANVAS_STATUS_BRUSH_ZOOMING) {
		auto dif = ((OriginPosition - pos).y());
		auto hd = float(dif) / float( viewport()->height());


		auto zdl = 0.0;
		if(CanvasState == CANVAS_STATUS_ZOOMING) {
			if(hd > 0) 	zdl = OriginZoom + (8 * hd);
			else 		zdl = OriginZoom + ((OriginZoom * 0.9) * hd);
			SetZoom(zdl);

		} else if (CanvasState == CANVAS_STATUS_BRUSH_ZOOMING) {
			if(hd > 0)	zdl = OriginZoom + floor(30 * hd);
			else		zdl = OriginZoom + floor((OriginZoom * 0.9) * hd);
			CurrentBrush->SetWidth(zdl);
			emit CursorChange(CurrentBrush);
		}
		viewport()->update();

	}

	CurrentBrush->SetPosition(RealPosition(pos));
	CurrentBrush->SetPressure(pressure);
	
	emit PenMove(CurrentBrush);


	OldPosition = pos;
	viewport()->update();
}

bool ParupaintCanvasView::OnScroll(const QPointF & pos, Qt::KeyboardModifiers modifiers, float delta_y)
{
	if(CurrentBrush == nullptr) return false;

	float actual_zoom = delta_y / 120.0;
	if((modifiers & Qt::ControlModifier) || CanvasState == CANVAS_STATUS_MOVING){
		AddZoom(actual_zoom * 0.2);
		
	} else {
		float new_size = CurrentBrush->GetWidth() + (actual_zoom*4);
		CurrentBrush->SetWidth(new_size);
		emit CursorChange(CurrentBrush);

	}
	CurrentBrush->SetPosition(RealPosition(OldPosition));
	emit PenMove(CurrentBrush);
	viewport()->update();
	return false;
}

bool ParupaintCanvasView::OnKeyDown(QKeyEvent * event)
{
	if(CurrentBrush == nullptr) return false;

	if(event->key() == Qt::Key_Space){
		if((event->modifiers() & Qt::ShiftModifier) &&
			CanvasState != CANVAS_STATUS_BRUSH_ZOOMING){
			
			CanvasState = CANVAS_STATUS_BRUSH_ZOOMING;
			OriginZoom = CurrentBrush->GetWidth();
			OriginPosition = OldPosition;
			return true;

		} else if((event->modifiers() & Qt::ControlModifier) &&
			CanvasState != CANVAS_STATUS_ZOOMING) {

			CanvasState = CANVAS_STATUS_ZOOMING;
			OriginZoom = GetZoom();
			OriginPosition = OldPosition;
			return true;

		} else if(CanvasState != CANVAS_STATUS_MOVING){
			CanvasState = CANVAS_STATUS_MOVING;
		}
	}
	return false; // Let keys go by default
}

bool ParupaintCanvasView::OnKeyUp(QKeyEvent * event)
{
	if(CurrentBrush == nullptr) return false;
	
	if(event->key() == Qt::Key_Control) {
		if(CanvasState == CANVAS_STATUS_ZOOMING){
			CanvasState = CANVAS_STATUS_MOVING;
		}
	}
	if(event->key() == Qt::Key_Shift) {
		if(CanvasState == CANVAS_STATUS_BRUSH_ZOOMING){
			CanvasState = CANVAS_STATUS_MOVING;
		}
	}
	if(event->key() == Qt::Key_Space && 
		(CanvasState != CANVAS_STATUS_IDLE)){
		CanvasState = CANVAS_STATUS_IDLE;
	}
	// Mini-rant.
	// Okay, so Qt seems to fire the key events while you're holding it.
	// That's fine by me, because it has a flag for auto repeat. But for some 
	// reason the flag sometimes flips around, seemingly randomly?! ?!?!?!
	// I've tried really hard to find out the reason, but i have
	// not found it. I've only found one or two obscure forum posts, and
	// i'm not even sure if this is the thing they're talking about.
	// I tried different Qt version as well, but they all seem to have the
	// same problem.... Tried Qt5 and 4, same problems.
	//
	// Whatever.
	// Just gotta optimize this for that... 'bug'.
	OriginPosition = OldPosition;
	return false;
}


// Other events

void ParupaintCanvasView::OnCanvasUpdate()
{
	viewport()->update();
}


// ...

QPointF ParupaintCanvasView::RealPosition(const QPointF &pos)
{
	double tmp;
	qreal xf = qAbs(modf(pos.x(), &tmp));
	qreal yf = qAbs(modf(pos.y(), &tmp));

	QPoint p0(floor(pos.x()), floor(pos.y()));
	QPointF p1 = mapToScene(p0);
	QPointF p2 = mapToScene(p0 + QPoint(1,1));

	QPointF mapped(
		(p1.x()-p2.x()) * xf + p2.x(),
		(p1.y()-p2.y()) * yf + p2.y()
	);

	return mapped;	
}


// Qt events


void ParupaintCanvasView::drawForeground(QPainter *painter, const QRectF & )
{
	if(CurrentBrush == nullptr) return;

	if(CanvasState == CANVAS_STATUS_BRUSH_ZOOMING){
		auto ww = CurrentBrush->GetWidth();
		auto cc = CurrentBrush->GetColor();

		painter->save();

		auto tt = QColor(cc.red(), cc.green(), cc.blue(), 70);
		QPen pen(	QBrush(tt), 		ww, Qt::SolidLine, Qt::RoundCap);
		QPen pen_small(	QBrush(QColor(0, 0, 0)), 1,  Qt::SolidLine, Qt::RoundCap);
		
		pen_small.setCosmetic(true);
		
		painter->setPen(pen);
		painter->drawLine(CurrentBrush->GetPosition(), RealPosition(OriginPosition));
		painter->setPen(pen_small);
		painter->drawLine(CurrentBrush->GetPosition(), RealPosition(OriginPosition));

		painter->restore();

	}
}

bool ParupaintCanvasView::viewportEvent(QEvent * event)
{
	if(event->type() == QEvent::TabletPress){
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);

		PenState = PEN_STATE_TABLET_DOWN;
		tevent->accept();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
			OnPenDown(tevent->posF(),
			tevent->buttons(),
#else
			OnPenDown(tevent->pos(),
			Qt::NoButton,
#endif
			tevent->modifiers(),
			tevent->pressure()
		);
		return true;

	} else if(event->type() == QEvent::TabletRelease){
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);
		PenState = PEN_STATE_UP;

		OnPenUp(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
			tevent->posF(),
			tevent->buttons()
#else
			tevent->pos(),
			Qt::NoButton
#endif
		);
		return true;

	} else if(event->type() == QEvent::TabletMove) {
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);

		tevent->accept();
		OnPenMove(
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
				tevent->posF(),
				tevent->buttons(),
#else
				tevent->pos(),
				Qt::NoButton,
#endif
				tevent->modifiers(),
				tevent->pressure()
				);
		return true;
	}
	return QGraphicsView::viewportEvent(event);
}

void ParupaintCanvasView::mouseMoveEvent(QMouseEvent * event)
{
	if(PenState != PEN_STATE_TABLET_DOWN){
		OnPenMove(event->pos(), event->buttons(), event->modifiers(), 1.0);
	}
	QGraphicsView::mouseMoveEvent(event);
}

void ParupaintCanvasView::mousePressEvent(QMouseEvent * event)
{
	if(PenState == PEN_STATE_TABLET_DOWN && event->buttons() != Qt::RightButton)
	{
		return;
	}

	PenState = PEN_STATE_MOUSE_DOWN;
	OnPenDown(event->pos(), event->button(), event->modifiers(), 1.0);
	QGraphicsView::mousePressEvent(event);
}
void ParupaintCanvasView::mouseReleaseEvent(QMouseEvent * event)
{
	if(PenState != PEN_STATE_MOUSE_DOWN){
		return;
	}
	OnPenUp(event->pos(), event->button());
	PenState = PEN_STATE_UP;
}

void ParupaintCanvasView::wheelEvent(QWheelEvent * event)
{
	auto angle = 
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
		event->angleDelta().y();
#else
		event->delta();
#endif
	if(!OnScroll(event->pos(), event->modifiers(), angle)){
		event->ignore();	
	}
}

void ParupaintCanvasView::keyPressEvent(QKeyEvent * event)
{
	if(!event->isAutoRepeat() && OnKeyDown(event)){
		return;
	}
	QGraphicsView::keyPressEvent(event);
}
void ParupaintCanvasView::keyReleaseEvent(QKeyEvent * event)
{
	if(!event->isAutoRepeat() && OnKeyUp(event)){
		return;
	}
	QGraphicsView::keyReleaseEvent(event);
}

void ParupaintCanvasView::enterEvent(QEvent * event)
{
	QGraphicsView::enterEvent(event);
}
void ParupaintCanvasView::leaveEvent(QEvent * event)
{
	QGraphicsView::leaveEvent(event);
}
