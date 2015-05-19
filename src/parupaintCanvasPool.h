#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include "parupaintCursorPool.h"
#include "parupaintCanvasStrokeObject.h"
#include <QGraphicsScene>
#include <QHash>

class ParupaintCanvasObject;
class ParupaintCursor;

class ParupaintCanvasPool : public QGraphicsScene, public ParupaintCursorPool
{
Q_OBJECT
	private:
	ParupaintCanvasObject * Canvas;
	QHash<ParupaintBrush *, ParupaintCanvasStrokeObject*> strokes;

	public:
	ParupaintCanvasPool(QObject *parent);
	ParupaintCanvasObject * GetCanvas();

	virtual void AddCursor(QString, ParupaintCursor *);


	ParupaintCanvasStrokeObject * NewBrushStroke(ParupaintBrush * brush);
	void EndBrushStroke(ParupaintBrush * brush);
	int GetNumBrushStrokes(ParupaintBrush * brush);
	int GetTotalStrokes();
	void ClearStrokes();

	private slots:
	void OnCanvasResize(QSize old_size, QSize new_size);

signals:
	void UpdateView();

};





#endif
