#ifndef PARUPAINTVISUALCANVAS_H
#define PARUPAINTVISUALCANVAS_H

#include "core/parupaintPanvas.h"

#include <QGraphicsObject>

class ParupaintVisualCanvas : public ParupaintPanvas, public QGraphicsItem
{
Q_OBJECT
	private:
	int current_layer;
	int current_frame;

	bool canvas_preview;

	QLine 	line_preview;
	qreal	line_thickness;
	QColor 	line_color;

	QPixmap canvas_cache;

	QPixmap checker_pixmap;
	QPixmap fillpreview_pixmap;

	QPointF pastepreview_pos;
	QPixmap pastepreview_pixmap;

	QTimer * flash_timeout;
	QTimer * fillpreview_timeout;

	signals:
	void onCurrentLayerFrameChange(int l, int f);

	private slots:
	void timeoutRedraw();

	public slots:
	void current_lf_update(int l, int f);

	public:
	ParupaintVisualCanvas(QGraphicsItem * = nullptr);

	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	void setPreviewLine(const QLine & = QLine(), qreal = 0.0, const QColor = QColor());

	void newCache();
	void redraw(QRect = QRect());

	void setFillPreview(const QImage & = QImage());
	void setCurrentLayerFrame(int l, int f, bool flash = true);
	void addCurrentLayerFrame(int lc, int fc, bool flash = true);
	void adjustCurrentLayerFrame(bool flash = false);

	void setPastePreview(const QImage & = QImage(), const QPointF & pos = QPointF());
	void setPastePreviewPosition(const QPointF &);
	bool hasPastePreview();

	int currentLayer() const;
	int currentFrame() const;

	ParupaintLayer * currentCanvasLayer();
	ParupaintFrame * currentCanvasFrame();

	void setPreview(bool b);
	bool isPreview();

	const QPixmap & canvasCache() const;
};

#endif
