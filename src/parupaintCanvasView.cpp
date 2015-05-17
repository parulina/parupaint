
#include <QScrollBar>
#include <QMouseEvent>
#include <QTabletEvent>
#include <QBitmap>

#include <QDebug>


#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"


//QGraphicsView for canvas view


ParupaintCanvasView::ParupaintCanvasView(QWidget * parent) : QGraphicsView(parent),
	// Canvas stuff
	CanvasState(CANVAS_STATUS_IDLE), PenState(PEN_STATE_UP), Zoom(1.0), Drawing(false),
	// Brush stuff
	CurrentBrush(&brush), BrushPosition(0, 0), Pressure(1.0),
	DrawButton(Qt::LeftButton), MoveButton(Qt::MiddleButton), SwitchButton(Qt::RightButton)
{
	// mouse pointers and canvas itself
	//
	
	setBackgroundBrush(QColor(200, 200, 200));
	viewport()->setMouseTracking(true);
	viewport()->setCursor(Qt::BlankCursor);
	CurrentBrush->SetSize(50);
	
	SetZoom(Zoom);
}
void ParupaintCanvasView::SetCanvas(ParupaintCanvasPool * canvas)
{
	setScene(canvas);
}

// called automatically by Qt's drawForeground.
void ParupaintCanvasView::DrawBrush(QPainter *painter)
{
	auto size = CurrentBrush->Size();
	
	painter->save();
	painter->setRenderHint(QPainter::Antialiasing, false);

	QPen pen(Qt::white), pen_down(Qt::gray);
	pen.setCosmetic(true);
	pen_down.setCosmetic(true);

	painter->setPen(pen);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	
	const QRectF cc((BrushPosition - QPointF(size/2, size/2)), 
				QSizeF(size, size));
	const QRectF cp((BrushPosition - QPointF((size*Pressure)/2, (size*Pressure)/2)),
				QSizeF(size*Pressure, size*Pressure));
	painter->drawEllipse(cc); // brush width

	if(PenState != PEN_STATE_UP){
		painter->setPen(pen_down);
		painter->drawEllipse(cp); // pressure
	}

	painter->restore();
	
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
	BrushPosition = RealPosition(pos);
	if(buttons == DrawButton && !Drawing){
		Drawing = true;

	} else if(buttons == MoveButton && CanvasState != CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_MOVING;
	}
	viewport()->update();
}

void ParupaintCanvasView::OnPenUp(const QPointF &pos, Qt::MouseButtons buttons)
{
	BrushPosition = RealPosition(pos);
	if(buttons == DrawButton && Drawing){
		Drawing = false;
	}
	if(buttons == MoveButton && CanvasState == CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_IDLE;
	}

	viewport()->update();
}

void ParupaintCanvasView::OnPenMove(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure)
{
	if(CanvasState == CANVAS_STATUS_MOVING){
		QScrollBar *ver = verticalScrollBar();
		QScrollBar *hor = horizontalScrollBar();
		auto dif = OldPosition - pos;
		hor->setSliderPosition(hor->sliderPosition() + dif.x());
		ver->setSliderPosition(ver->sliderPosition() + dif.y());
	}

	Pressure = pressure;
	OldPosition = pos;
	BrushPosition = RealPosition(OldPosition);
	

	viewport()->update();
}

bool ParupaintCanvasView::OnScroll(const QPointF & pos, Qt::KeyboardModifiers modifiers, QPoint delta)
{
	float actual_zoom = delta.y() / 120.0;
	if((modifiers & Qt::ControlModifier) || CanvasState == CANVAS_STATUS_MOVING){
		AddZoom(actual_zoom * 0.2);
		
	} else {
		float new_size = CurrentBrush->Size() + (actual_zoom*4);
		CurrentBrush->SetSize(new_size);

	}
	BrushPosition = RealPosition(pos);
	viewport()->update();
	return false;
}

bool ParupaintCanvasView::OnKeyDown(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space && CanvasState != CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_MOVING;
	}
	return true;
}

bool ParupaintCanvasView::OnKeyUp(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space && CanvasState == CANVAS_STATUS_MOVING){
		CanvasState = CANVAS_STATUS_IDLE;
	}
	return true;
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


void ParupaintCanvasView::drawForeground(QPainter *painter, const QRectF & rect)
{
	DrawBrush(painter);
}

bool ParupaintCanvasView::viewportEvent(QEvent * event)
{
	if(event->type() == QEvent::TabletPress){
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);

		PenState = PEN_STATE_TABLET_DOWN;
		tevent->accept();
		OnPenDown(tevent->posF(),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
			tevent->buttons(),
#else
			Qt::NoButton,
#endif
			tevent->modifiers(),
			tevent->pressure()
		);


	} else if(event->type() == QEvent::TabletRelease){
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);
		PenState = PEN_STATE_UP;

		OnPenUp(tevent->posF(),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
			tevent->buttons()
#else
			Qt::NoButton
#endif
		);
	

	} else if(event->type() == QEvent::TabletMove) {
		QTabletEvent *tevent = static_cast<QTabletEvent*>(event);

		tevent->accept();
		OnPenMove(tevent->posF(),
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
				tevent->buttons(),
#else
				Qt::NoButton,
#endif
				tevent->modifiers(),
				tevent->pressure()
				);
	} else {
		return QGraphicsView::viewportEvent(event);
	}
	return true;
}

void ParupaintCanvasView::mouseMoveEvent(QMouseEvent * event)
{
	if(PenState != PEN_STATE_TABLET_DOWN){
		OnPenMove(event->pos(), event->buttons(), event->modifiers(), 1.0);
	}
}

void ParupaintCanvasView::mousePressEvent(QMouseEvent * event)
{
	if(PenState != PEN_STATE_UP){
		return;
	}
	PenState = PEN_STATE_MOUSE_DOWN;
	OnPenDown(event->pos(), event->button(), event->modifiers(), 1.0);
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
	if(OnScroll(event->pos(), event->modifiers(), event->angleDelta())){
		QGraphicsView::wheelEvent(event);
	}
}

void ParupaintCanvasView::keyPressEvent(QKeyEvent * event)
{
	if(OnKeyDown(event)){
		QGraphicsView::keyPressEvent(event);
	}
}
void ParupaintCanvasView::keyReleaseEvent(QKeyEvent * event)
{
	if(OnKeyUp(event)){
		QGraphicsView::keyReleaseEvent(event);
	}
}
