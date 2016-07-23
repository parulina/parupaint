#include "parupaintCanvasView.h"

// ParupaintCanvasView handles the view of the canvas:
// pixel grid, scrollbars, zoom, etc

#include <QDebug>
#include <QSettings>
#include <QTimer>
#include <QtMath>
#include <QDate> // new year event

#include "widget/parupaintScrollBar.h"

inline QString ordinalSuffix(int i){
	int j = (i % 10), k = (i % 100);
	if(j == 1 && k != 11) return "st";
	if(j == 2 && k != 12) return "nd";
	if(j == 3 && k != 13) return "rd";
	else return "th";
}

ParupaintCanvasView::ParupaintCanvasView(QWidget * parent) : QGraphicsView(parent),
	toast_timer(new QTimer(this)),
	tablet_active(false),
	canvas_horflip(false), canvas_verflip(false)
{
	this->setObjectName("CanvasView");
	this->setAcceptDrops(false);

	// Cursors move quickly, creating artifacts
	// The visualcursor bounds are correct and everything, but
	// for some reason when the cursor moves around quickly
	// the graphicsview lags behind.
	//this->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);

	this->setFocusPolicy(Qt::WheelFocus);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	ParupaintScrollBar * vbar = new ParupaintScrollBar(Qt::Vertical, this, true);
	ParupaintScrollBar * hbar = new ParupaintScrollBar(Qt::Horizontal, this, true);
	this->setVerticalScrollBar(vbar);
	this->setHorizontalScrollBar(hbar);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	connect(vbar, &ParupaintScrollBar::directionMove, this, &ParupaintCanvasView::scrollbarMove);
	connect(hbar, &ParupaintScrollBar::directionMove, this, &ParupaintCanvasView::scrollbarMove);

	//setBackgroundBrush(QColor(200, 200, 200));
	this->setBackgroundBrush(QBrush(Qt::white, Qt::NoBrush));


	viewport()->setMouseTracking(true);
	
	QSettings cfg;
	this->setPixelGrid(cfg.value("client/pixelgrid", true).toBool());
	this->setSmoothZoom(cfg.value("client/smoothzoom", true).toBool());
	cfg.setValue("client/pixelgrid", this->pixelGrid());
	cfg.setValue("client/smoothzoom", this->smoothZoom());

	this->setViewportCursor(cfg.value("client/cursor", true).toBool());
	this->setFastViewport(cfg.value("client/fastviewport", false).toBool());

	connect(toast_timer, &QTimer::timeout, this, &ParupaintCanvasView::toastTimeout);
	this->showToast("");

	QDate date = QDate::currentDate();
	QRect pattern_size;
	if((date.month() == 12 && date.dayOfYear() == 364)){
		pattern_size.setRect(0, 0, 64, 32);
		this->showToast("♪ HAVE A HAPPY NEW YEAR ♪", 0);

	} else if((date.month() == 11 && date.day() == 5)){
		pattern_size.setRect(0, 32, 64, 32);
		int num = (date.year() - 2012);
		if(num > 0){
			QString suf = ordinalSuffix(num).toUpper();
			QString str = QString("~HAPPY %1%2 BIRTHDAY, PARUPAINT!!~").arg(num).arg(suf);
			this->showToast(str, 0);
		}
	}

	if(!pattern_size.isNull()){
		QImage patterns(":/resources/patterns.png");
		QBrush brush(patterns.copy(pattern_size));
		this->setBackgroundBrush(brush);
	}

	this->setZoom(1.0);
	this->show();
}

void ParupaintCanvasView::showToast(const QString & text, qreal timeout)
{
	toast_timer->setSingleShot(true);
	toast_timer->stop();

	if(text.length() == 0 || timeout < 0) return;
	if(timeout == 0) toast_timer->setSingleShot(false);
	if(timeout > 0) {
		toast_timer->start(timeout);
	}
	this->toast_message = text;
	this->viewport()->update();
}

void ParupaintCanvasView::resetFlip()
{
	canvas_horflip = canvas_verflip = false;
	this->setZoom(this->zoom());
}
void ParupaintCanvasView::flipView(bool h, bool v)
{
	if(h) canvas_horflip = !canvas_horflip;
	if(v) canvas_verflip = !canvas_verflip;

	this->setZoom(this->zoom());
}
bool ParupaintCanvasView::isFlipped()
{
	return (canvas_horflip || canvas_verflip);
}

void ParupaintCanvasView::moveView(const QPointF & move)
{
	QScrollBar * hor = this->horizontalScrollBar();
	QScrollBar * ver = this->verticalScrollBar();
	hor->setSliderPosition(hor->sliderPosition() + move.x());
	ver->setSliderPosition(ver->sliderPosition() + move.y());
}

void ParupaintCanvasView::resetView()
{
	this->setZoom(1.0);
	this->resetTransform();
}

void ParupaintCanvasView::toastTimeout()
{
	this->viewport()->update();
}
void ParupaintCanvasView::scrollbarMove(const QPoint & move)
{
	QPoint move_added(move.x() * this->zoom(), move.y() * this->zoom());
	this->moveView(move_added);
}

// getters
qreal ParupaintCanvasView::zoom()
{
	return canvas_zoom;
}
bool ParupaintCanvasView::smoothZoom()
{
	return smooth_zoom;
}
bool ParupaintCanvasView::pixelGrid()
{
	return pixel_grid;
}

// setters
void ParupaintCanvasView::setZoom(qreal z)
{
	if(z < 0.2) z = 0.2;
	canvas_zoom = z;

	if(smooth_zoom) this->setRenderHint(QPainter::SmoothPixmapTransform, !(z > 3));
	
	QMatrix nm(1,0,0,1, matrix().dx(), matrix().dy());

	const qreal opt_zoom = (canvas_zoom < 1 ? (qRound(canvas_zoom * 10.0) / 10.0): canvas_zoom);
	nm.scale(opt_zoom, opt_zoom);

	nm.scale(canvas_horflip ? -1 : 1, canvas_verflip ? -1 : 1);
	setMatrix(nm);
}
void ParupaintCanvasView::setSmoothZoom(bool sz)
{
	smooth_zoom = sz;
}
void ParupaintCanvasView::setPixelGrid(bool pg)
{
	pixel_grid = pg;
	this->viewport()->update();
}
void ParupaintCanvasView::setViewportCursor(bool cursor)
{
 	this->setCursor(cursor ? Qt::CrossCursor : Qt::BlankCursor);
	this->viewport()->update();
}
void ParupaintCanvasView::setFastViewport(bool fast)
{
	this->setViewportUpdateMode(fast ?
			QGraphicsView::FullViewportUpdate :
			QGraphicsView::MinimalViewportUpdate);
	this->viewport()->update();
}

// various
void ParupaintCanvasView::addZoom(qreal z)
{
	this->setZoom(this->zoom() + z);
}

inline QPointF mapToSceneF(QGraphicsView * view, const QPointF & pos)
{
	double tmp;
	qreal xf = qAbs(modf(pos.x(), &tmp));
	qreal yf = qAbs(modf(pos.y(), &tmp));

	QPoint p0(qFloor(pos.x()), qFloor(pos.y()));
	QPointF p1 = view->mapToScene(p0);
	QPointF p2 = view->mapToScene(p0 + QPoint(1, 1));

	return QPointF(
		(p1.x() - p2.x()) * xf + p2.x(),
		(p1.y() - p2.y()) * yf + p2.y()
	);
}
void ParupaintCanvasView::showEvent(QShowEvent * event)
{
	QGraphicsView::showEvent(event);
}

void ParupaintCanvasView::tabletEvent(QTabletEvent * event)
{
	if(this->verticalScrollBar()->isSliderDown()) return;
	if(this->horizontalScrollBar()->isSliderDown()) return;

	pen_info.old_pos = pen_info.pos;
	pen_info.pos = mapToSceneF(this, event->pos());

	pen_info.old_gpos = pen_info.gpos;
	pen_info.gpos = event->globalPos();

	if(event->pointerType() != pen_info.pointer){
		pen_info.pointer = event->pointerType();
		emit pointerPointer(pen_info);
	}
	pen_info.buttons = event->buttons();
	pen_info.modifiers = event->modifiers();
	pen_info.pressure = event->pressure();

	if(event->type() == QEvent::TabletRelease){
		if(event->pointerType() != QTabletEvent::Cursor) tablet_active = false;
		emit pointerRelease(pen_info);
	}
	if(event->type() == QEvent::TabletPress){
		if(event->pointerType() != QTabletEvent::Cursor) tablet_active = true;
		emit pointerPress(pen_info);
	}
	if(event->type() == QEvent::TabletMove){
		emit pointerMove(pen_info);
	}
}

// following functions redirect mouse events to tablet events.

inline QTabletEvent MouseToTabletEvent(QEvent::Type type, QMouseEvent * event){
	return QTabletEvent(type, event->localPos(), event->globalPos(),
			QTabletEvent::NoDevice,
			QTabletEvent::Cursor,
			(event->buttons() != 0), // pressure
			0, 0, 0, 0, 0, // xtilt, ytilt, tang_rot, rot, z
			event->modifiers(), 0, event->button(), event->buttons());

}

void ParupaintCanvasView::mouseDoubleClickEvent(QMouseEvent * event)
{
	this->ParupaintCanvasView::mousePressEvent(event);
}
void ParupaintCanvasView::mousePressEvent(QMouseEvent * event)
{
	if(tablet_active) return;

	QTabletEvent tablet = MouseToTabletEvent(QEvent::TabletPress, event);
	this->tabletEvent(&tablet);

	event->accept();
}
void ParupaintCanvasView::mouseReleaseEvent(QMouseEvent * event)
{
	if(tablet_active) return;

	QTabletEvent tablet = MouseToTabletEvent(QEvent::TabletRelease, event);
	this->tabletEvent(&tablet);

	event->accept();
}
void ParupaintCanvasView::mouseMoveEvent(QMouseEvent * event)
{
	if(tablet_active) return;

	QTabletEvent tablet = MouseToTabletEvent(QEvent::TabletMove, event);
	this->tabletEvent(&tablet);

	event->accept();
	this->QGraphicsView::mouseMoveEvent(event);
}
void ParupaintCanvasView::wheelEvent(QWheelEvent * event)
{
	// angle = event->angleDelta().y();
	emit pointerScroll(event);
}

void ParupaintCanvasView::drawForeground(QPainter * painter, const QRectF & rect)
{
	// TODO move to canvasscene
	if(this->pixelGrid() && this->zoom() > 8.0 && true){

		// to prevent edge stuff fucking up with brush
		QRect pixel_rect = rect.toRect().adjusted(-1, -1, 1, 1);

		QPen grid_pen(Qt::gray);
		grid_pen.setCosmetic(true);
		painter->setPen(grid_pen);

		for(int x = pixel_rect.left(); x <= rect.right(); ++x){
			painter->drawLine(x, pixel_rect.top(), x, pixel_rect.bottom()+1);
		}
		for(int y = pixel_rect.top(); y <= rect.bottom(); ++y){
			painter->drawLine(pixel_rect.left(), y, pixel_rect.right()+1, y);
		}
	}
}

void ParupaintCanvasView::paintEvent(QPaintEvent * event)
{
	QGraphicsView::paintEvent(event);

	if(toast_timer->isActive() || (!toast_timer->isSingleShot() && !toast_timer->isActive())){

		QPainter painter(viewport());
		QFont font = painter.font();
		font.setPointSize(15);
		painter.setFont(font);

		const QTransform & t = painter.transform();
		painter.setTransform(QTransform(1.0, t.m12(), t.m13(), t.m21(), 1.0, t.m23(), t.m31(), t.m32(), t.m33()));

		QSize text_size = painter.fontMetrics().size(Qt::TextSingleLine, toast_message) + QSize(10, 4);
		QRectF rect(QPointF(this->width()/2 - text_size.width()/2, 0), text_size);
		painter.fillRect(rect, Qt::black);
		painter.drawText(rect, Qt::AlignCenter, toast_message);
	}
}
