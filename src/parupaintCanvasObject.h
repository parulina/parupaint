#ifndef PARUPAINTCANVASOBJECT_H
#define PARUPAINTCANVASOBJECT_H

#include "core/parupaintPanvas.h"
#include <QGraphicsObject>

class QTimer;

class ParupaintCanvasObject : public QGraphicsObject, public ParupaintPanvas
{
Q_OBJECT
	private:
	_lint CurrentLayer;
	_lint CurrentFrame;
	bool Preview;

	QPixmap checker;
	QPixmap cache;

	QTimer * flash_timeout;

	void NewCache();

	public:
	~ParupaintCanvasObject();
	ParupaintCanvasObject();
	virtual void New(QSize s, _lint l, _fint f);
	virtual void Resize(QSize);
	
	bool IsPreview() const;
	void SetPreview(bool);
	void TogglePreview();

	void SetLayerFrame(bool, _lint, _fint =0);
	void AddLayerFrame(bool, int, int);
	void FixLayerFrame();
	_lint GetCurrentLayer();
	_fint GetCurrentFrame();

	void RedrawCache();
	void RedrawCache(QRect);
	const QPixmap & GetCache() const;


	QRectF boundingRect() const;
	protected:
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	signals:
	void ResizeSignal(QSize, QSize);
	void CurrentSignal(int, int);
};

#endif
