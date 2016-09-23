#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QStyledItemDelegate>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QAbstractScrollArea>

class ParupaintCanvasModel;
class ParupaintFlayerControl;
class ParupaintVisualCanvas;
class ParupaintPanvas;

class ParupaintFlayerPainter : public QStyledItemDelegate
{
Q_OBJECT
	public:
	ParupaintFlayerPainter(QObject * = nullptr);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class ParupaintFixedViewport : public QAbstractScrollArea
{
Q_OBJECT
	protected: bool viewportEvent(QEvent * event);
	public: ParupaintFixedViewport(QWidget * = nullptr);
	public: Q_SIGNAL void contentsScrolledBy(qreal, qreal);
};

class ParupaintFlayerTopHeader : public QHeaderView
{
Q_OBJECT
	protected:
	void paintSection(QPainter *painter, const QRect & rect, int logicalIndex) const;
	public:
	ParupaintFlayerTopHeader(QWidget * = nullptr);
};

class ParupaintFlayerControlHeader : public QHeaderView
{
Q_OBJECT
	QList<ParupaintFlayerControl *> layer_controls;

	protected:
	void fixControlPositions();
	void paintSection(QPainter *painter, const QRect & rect, int logicalIndex) const;

	public:
	ParupaintFlayerControlHeader(QWidget * = nullptr);
	void setModel(QAbstractItemModel * model);

	Q_SLOT void layoutChange(const QList<QPersistentModelIndex> & parents);
	Q_SLOT void headerDataChange(Qt::Orientation orientation, int first, int last);
};

class ParupaintFlayer : public QTableView
{
Q_OBJECT
	private:
	QPoint 	old_pos;

	void selectLayerFrameItem(const QModelIndex & index);
	int modelLayer(int layer);

	private slots:
	void itemActivated(const QModelIndex & current, const QModelIndex & previous);

	signals:
	void onLayerFrameSelect(int layer, int frame);

	public:
	ParupaintFlayer(QWidget * = nullptr);
	void setCanvasModel(ParupaintCanvasModel * model);
	void selectLayerFrame(int layer, int frame);

	protected:
	void mouseMoveEvent(QMouseEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void resizeEvent(QResizeEvent * event);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif
