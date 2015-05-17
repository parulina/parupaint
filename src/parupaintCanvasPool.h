#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include "parupaintCursorPool.h"
#include <QGraphicsScene>
#include <QHash>
class ParupaintCanvasObject;
class ParupaintCursor;

class ParupaintCanvasPool : public QGraphicsScene, public ParupaintCursorPool
{
Q_OBJECT
	private:
	ParupaintCanvasObject * Canvas;

	public:
	ParupaintCanvasPool(QObject *parent);
	ParupaintCanvasObject * GetCanvas();

	virtual void AddCursor(QString, ParupaintCursor *);

	private slots:
	void OnCanvasResize(QSize old_size, QSize new_size);

signals:
	void UpdateView();

};





#endif
