#ifndef PARUPAINTCANVASVIEW_H
#define PARUPAINTCANVASVIEW_H

#include "parupaintCanvasBrush.h"

#include <QGraphicsView>
class QWidget;
class QPainter;

class ParupaintCanvasPool;


enum PenStatus {
	PEN_STATE_UP,
	PEN_STATE_MOUSE_DOWN,
	PEN_STATE_TABLET_DOWN
};

class ParupaintCanvasView : public QGraphicsView {
Q_OBJECT
	private:
	ParupaintCanvasBrush brush;
	ParupaintCanvasBrush * CurrentBrush;
	
	QPointF 	BrushPosition;
	float		Pressure;
	PenStatus	PenState;
	bool		Drawing;


	protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	void drawForeground(QPainter *painter, const QRectF& rect);
	bool viewportEvent(QEvent *event);

	public:
	ParupaintCanvasView(QWidget* parent);
	void SetCanvas(ParupaintCanvasPool * canvas);
	void DrawBrush(QPainter *painter);


	QPointF RealPosition(const QPointF &pos);

	void OnScroll(const QPointF & pos, Qt::KeyboardModifiers modifiers, QPoint delta);
	void OnPenDown(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure);
	void OnPenUp(const QPointF &pos, Qt::MouseButtons buttons);
	void OnPenMove(const QPointF &pos, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, float pressure);

	// set/reset zoom, rotation, etc...
//	void UpdateTitle();
};





#endif
