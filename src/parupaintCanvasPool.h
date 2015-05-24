#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include "parupaintCanvasBrushPool.h"
#include "parupaintCanvasStrokeObject.h"
#include <QGraphicsScene>
#include <QMultiMap>

class ParupaintCanvasObject;
class ParupaintCanvasBrush;

class ParupaintCanvasPool : public QGraphicsScene, public ParupaintCanvasBrushPool
{
Q_OBJECT
	private:
	ParupaintCanvasObject * Canvas;
	QMultiMap<ParupaintBrush *, ParupaintCanvasStrokeObject*> strokes;

	public:
	ParupaintCanvasPool(QObject *parent);
	ParupaintCanvasObject * GetCanvas();

	virtual void AddCursor(QString, ParupaintCanvasBrush *);
	virtual void RemoveCursor(ParupaintCanvasBrush *);


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

	void TriggerViewUpdate();


	private slots:
	void OnCanvasResize(QSize old_size, QSize new_size);

signals:
	// Refresh the view.
	void UpdateView();
	// Canvas update (l/f..)
	void UpdateCanvas();

private slots:
	void CurrentChange(int, int);
};





#endif
