#ifndef PARUPAINTCANVASPOOL_H
#define PARUPAINTCANVASPOOL_H

#include <QGraphicsScene>
#include <QHash>
class ParupaintCanvasObject;
class ParupaintCanvasBrush;

class ParupaintCanvasPool : public QGraphicsScene
{
Q_OBJECT
	private:
	QHash<int, ParupaintCanvasBrush*> Cursors;
	ParupaintCanvasObject * Canvas;

//	CurrentFrame
	public:
	ParupaintCanvasPool(QObject *parent);
	void ClearCursors();
	ParupaintCanvasObject * GetCanvas();

	private slots:
	void OnCanvasResize(QSize old_size, QSize new_size);

signals:
	void UpdateView();

};





#endif
