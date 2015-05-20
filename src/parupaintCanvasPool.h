#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include "parupaintCursorPool.h"
#include "parupaintCanvasStrokeObject.h"
#include <QGraphicsScene>
#include <QMultiMap>

class ParupaintCanvasObject;
class ParupaintCursor;

class ParupaintCanvasPool : public QGraphicsScene, public ParupaintCursorPool
{
Q_OBJECT
	private:
	ParupaintCanvasObject * Canvas;
	QMultiMap<ParupaintBrush *, ParupaintCanvasStrokeObject*> strokes;

	public:
	ParupaintCanvasPool(QObject *parent);
	ParupaintCanvasObject * GetCanvas();

	virtual void AddCursor(QString, ParupaintCursor *);
	virtual void RemoveCursor(ParupaintCursor *);


	ParupaintCanvasStrokeObject * NewBrushStroke(ParupaintBrush * brush);
	void EndBrushStroke(ParupaintBrush * brush);

	void UpdateBrushStrokes(ParupaintBrush *);
	void UndoBrushStroke(ParupaintBrush *);
	void RedoBrushStroke(ParupaintBrush *);

	void SquashBrushStrokes(ParupaintBrush *);
	void ClearBrushStrokes(ParupaintBrush *);

	int GetNumBrushStrokes(ParupaintBrush * );
	int GetTotalStrokes();
	void ClearStrokes();

	private slots:
	void OnCanvasResize(QSize old_size, QSize new_size);

signals:
	void UpdateView();

private slots:
	void CurrentChange(int, int);
};





#endif
