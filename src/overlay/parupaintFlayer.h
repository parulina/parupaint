#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QStyledItemDelegate>
#include <QTableView>
#include <QComboBox>

class ParupaintCanvasModel;

class ParupaintVisualCanvas;
class ParupaintPanvas;

class FlayerComboBox : public QComboBox
{
Q_OBJECT
	private:
	void hidePopup();

	public:
	FlayerComboBox(QWidget * = nullptr);

	signals:
	void onPopupHide();
};

class ParupaintFlayerPainter : public QStyledItemDelegate
{
Q_OBJECT
	private:
	bool eventFilter(QObject * editor, QEvent * event);

	public:
	ParupaintFlayerPainter(QObject * = nullptr);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	QWidget * createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	void setEditorData(QWidget * editor, const QModelIndex & index) const;
	void setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const;

	Q_SLOT void commitEdit();
};

class ParupaintFlayer : public QTableView
{
Q_OBJECT
	private:
	QPoint 	old_pos;

	void selectLayerFrameItem(const QModelIndex & index);

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

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif
