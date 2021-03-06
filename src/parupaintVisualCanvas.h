#ifndef PARUPAINTVISUALCANVAS_H
#define PARUPAINTVISUALCANVAS_H

#include "core/parupaintPanvas.h"
#include "parupaintCheckerboardPixmap.h"

#include <QAbstractTableModel>
#include <QGraphicsObject>


class ParupaintVisualCanvas;
class ParupaintCanvasModel : public QAbstractTableModel
{
Q_OBJECT
	public:
	enum roles {
		LayerVisibleRole = Qt::UserRole,
		LayerModeRole = Qt::UserRole+1,
		LayerNameRole = Qt::UserRole+2
	};
	ParupaintCanvasModel(ParupaintVisualCanvas * panvas);
	
	int rowCount(const QModelIndex & index = QModelIndex()) const;
	int columnCount(const QModelIndex & index = QModelIndex()) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
	bool setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role);
	Qt::ItemFlags flags(const QModelIndex & index) const;

	Q_SLOT void updateLayout();
	Q_SLOT void updateLayer(int l);

	Q_SIGNAL void onLayerVisibilityChange(int layer, bool visible);
	Q_SIGNAL void onLayerModeChange(int layer, int mode);
	Q_SIGNAL void onLayerNameChange(int layer, QString name);
};

class ParupaintVisualCanvas : public ParupaintPanvas, public QGraphicsItem
{
Q_OBJECT
	private:
	ParupaintCanvasModel canvas_model;
	int current_layer;
	int current_frame;

	bool canvas_preview;

	QLine 	line_preview;
	qreal	line_thickness;
	QColor 	line_color;

	QPixmap canvas_cache;

	ParupaintCheckerboardPixmap checker_pixmap;
	QPixmap fillpreview_pixmap;

	QPointF pastepreview_pos;
	QPixmap pastepreview_pixmap;

	QTimer * flash_timeout;
	QTimer * fillpreview_timeout;

	QTimer * play_timer;

	signals:
	void onCurrentLayerFrameChange(int l, int f);

	private slots:
	void timeoutRedraw();

	public:
	ParupaintVisualCanvas(QGraphicsItem * = nullptr);

	QRectF boundingRect() const;
	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	void play();
	void stop();
	void togglePlay();
	bool isPlaying();
	Q_SLOT void updatePlayTimer();

	void setPreviewLine(const QLine & = QLine(), qreal = 0.0, const QColor = QColor());

	void newCache();
	void redraw(QRect = QRect());

	void setFillPreview(const QImage & = QImage());
	void setCurrentLayerFrame(int l, int f);
	void setCurrentLayerFrame(int l, int f, bool flash);
	void addCurrentLayerFrame(int lc, int fc, bool flash = true);
	void adjustCurrentLayerFrame(bool flash = false);
	void nextFrame();

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

	ParupaintCanvasModel * model();
};

#endif
