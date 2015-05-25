#ifndef PARUPAINTCANVASOBJECT_H
#define PARUPAINTCANVASOBJECT_H

#include "core/parupaintPanvas.h"

#include <QGraphicsObject>


class ParupaintCanvasObject : public QGraphicsObject, public ParupaintPanvas
{
Q_OBJECT
	private:
	_lint CurrentLayer;
	_lint CurrentFrame;
	bool Preview;

	QPixmap checker;
	QPixmap cache;

	void RedrawCache();

	public:
	ParupaintCanvasObject();
	virtual void New(QSize s, _lint l, _fint f);
	virtual void Resize(QSize);
	
	bool IsPreview() const;
	void SetPreview(bool);
	void TogglePreview();

	void SetLayerFrame(_lint, _fint =0);
	void AddLayerFrame(int, int);
	_lint GetCurrentLayer();
	_fint GetCurrentFrame();

	void TriggerCacheRedraw();


	QRectF boundingRect() const;
	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	signals:
	void ResizeSignal(QSize, QSize);
	void CurrentSignal(int, int);
};

#endif
