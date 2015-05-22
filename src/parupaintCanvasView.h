#ifndef PARUPAINTCANVASVIEW_H
#define PARUPAINTCANVASVIEW_H

#include <QGraphicsView>
class QWidget;
class QPainter;

class ParupaintBrush;
class ParupaintCanvasPool;
class ParupaintCanvasBrush;


enum PenStatus {
	PEN_STATE_UP,
	PEN_STATE_MOUSE_DOWN,
	PEN_STATE_TABLET_DOWN
};
enum CanvasStatus {
	CANVAS_STATUS_IDLE,
	CANVAS_STATUS_MOVING,
	CANVAS_STATUS_ZOOMING,
	CANVAS_STATUS_BRUSH_ZOOMING
};

class ParupaintCanvasView : public QGraphicsView {
Q_OBJECT
	private:
	ParupaintCanvasPool *CurrentCanvas;
	CanvasStatus	CanvasState;
	PenStatus	PenState;
	QPointF		OldPosition;
	QPointF		OriginPosition; // origin for zoom/bzoom
	float		OriginZoom;

	float		Zoom;
	bool		Drawing;

	ParupaintCanvasBrush* CurrentBrush;

	Qt::MouseButton DrawButton;
	Qt::MouseButton MoveButton;
	Qt::MouseButton SwitchButton;


	protected:
	void enterEvent(QEvent * event);
	void leaveEvent(QEvent * event);
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent * event);

	void wheelEvent(QWheelEvent* event);
	void drawForeground(QPainter *painter, const QRectF& rect);
	bool viewportEvent(QEvent *event);

	public:
	ParupaintCanvasView(QWidget* parent);
	void SetCanvas(ParupaintCanvasPool * canvas);
	void SetCurrentBrush(ParupaintBrush * brush);

	float GetZoom() const;
	void SetZoom(float z);
	void AddZoom(float z);


	QPointF RealPosition(const QPointF &pos);

	bool OnScroll(const QPointF & pos, Qt::KeyboardModifiers modifiers, float);
	void OnPenDown(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure);
	void OnPenUp(const QPointF &pos, Qt::MouseButtons buttons);
	void OnPenMove(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure);
	bool OnKeyDown(QKeyEvent * event);
	bool OnKeyUp(QKeyEvent * event);
	// set/reset zoom, rotation, etc...
//	void UpdateTitle();


	signals:
	void PenDrawStart(ParupaintBrush *);
	void PenMove(ParupaintBrush *);
	void PenDrawStop(ParupaintBrush *);

	void CursorChange(ParupaintBrush *);

public slots:
	void OnCanvasUpdate();

};





#endif
