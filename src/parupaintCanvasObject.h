#ifndef PARUPAINTCANVASOBJECT_H
#define PARUPAINTCANVASOBJECT_H

#include "panvas/parupaintPanvas.h"

#include <QGraphicsObject>


class ParupaintCanvasObject : public QGraphicsObject, public ParupaintPanvas
{
Q_OBJECT
	private:
	_lint CurrentLayer;
	_lint CurrentFrame;

	public:
	ParupaintCanvasObject();
	virtual void New(QSize s, _lint l, _fint f);
	virtual void Resize(QSize);
	void SetLayerFrame(_lint, _fint =0);
	void AddLayerFrame(int, int);
	_lint GetCurrentLayer();
	_fint GetCurrentFrame();



	QRectF boundingRect() const;
	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	signals:
	void ResizeSignal(QSize, QSize);
	void CurrentSignal(int, int);
};

#endif
