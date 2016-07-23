#ifndef PARUPAINTCANVASVIEW_H
#define PARUPAINTCANVASVIEW_H

#include <QGraphicsView>
#include <QTabletEvent>

class QPainter; // drawForeground
class QTimer; // toast_timer
class QWidget;

typedef QTabletEvent::PointerType QPointerType;
typedef Qt::MouseButton QMouseButton;

struct penInfo {
	QPointF pos, old_pos;
	QPointF gpos, old_gpos;
	QPointerType pointer;
	Qt::MouseButtons buttons;
	Qt::KeyboardModifiers modifiers;
	qreal pressure;
};

class ParupaintCanvasView : public QGraphicsView {
Q_OBJECT
	private:
	Q_PROPERTY(qreal canvas_zoom READ zoom WRITE setZoom NOTIFY zoomChange)
	Q_PROPERTY(bool pixel_grid READ pixelGrid WRITE setPixelGrid NOTIFY pixelGridChange)
	Q_PROPERTY(bool smooth_zoom READ smoothZoom WRITE setSmoothZoom)

	QTimer *toast_timer;
	QString toast_message;

	qreal canvas_zoom;
	bool pixel_grid;
	bool smooth_zoom;

	QPointerType	pointer_type;
	QPointF		pointer_oldpos;
	penInfo		pen_info;
	bool		tablet_active;

	bool 		canvas_horflip, canvas_verflip;

	signals:
	void zoomChange(qreal);
	void pixelGridChange(bool);

	void pointerRelease(const penInfo & info);
	void pointerPress(const penInfo & info);
	void pointerMove(const penInfo & info);
	void pointerPointer(const penInfo & info);
	void pointerScroll(QWheelEvent *);

	private slots:
	void toastTimeout();
	void scrollbarMove(const QPoint&);

	public:
	qreal zoom();
	bool smoothZoom();
	bool pixelGrid();
	const QPixmap & pastePreview();

	void setZoom(qreal z);
	void setSmoothZoom(bool sz);
	void setPixelGrid(bool pg);

	void addZoom(qreal z);

	// -1 = hide immediately
	void showToast(const QString & text, qreal timeout = 3000);

	void resetFlip();
	void flipView(bool h, bool v);
	bool isFlipped();

	void moveView(const QPointF & move);
	void resetView();


	protected:
	void tabletEvent(QTabletEvent *event);
	void showEvent(QShowEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent * event);
	void drawForeground(QPainter *painter, const QRectF& rect);
	void paintEvent(QPaintEvent * event);

	// not sure why these are needed...
	void enterEvent(QEvent * event){ QGraphicsView::enterEvent(event); }
	void leaveEvent(QEvent * event){ QGraphicsView::leaveEvent(event); }

	public:
	ParupaintCanvasView(QWidget* parent);
};

#endif
