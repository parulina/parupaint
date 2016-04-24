#include "parupaintFlayer.h"

#include <QPainter>
#include <QHeaderView>
#include <QMouseEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QDebug>

#include "../parupaintVisualCanvas.h"
#include "../widget/parupaintScrollBar.h"
#include "../core/parupaintLayerModes.h"

FlayerComboBox::FlayerComboBox(QWidget * parent) :
	QComboBox(parent)
{
}

void FlayerComboBox::hidePopup()
{
	// this always clears the focus so that the cel
	// is not kept focused when clicking out of popup
	this->clearFocus();
	QComboBox::hidePopup();
}


ParupaintFlayerPainter::ParupaintFlayerPainter(QObject * parent) :
	QStyledItemDelegate(parent)
{
}
bool ParupaintFlayerPainter::eventFilter(QObject * editor, QEvent * event)
{
	if(event->type() == QEvent::KeyPress) {
		QKeyEvent * key_event = static_cast<QKeyEvent*>(event);
		if(key_event && (key_event->key() == Qt::Key_Tab || key_event->key() == Qt::Key_Backtab)){
			return false;
		}
	}
	return QStyledItemDelegate::eventFilter(editor, event);
}
void ParupaintFlayerPainter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if(index.column() >= 3){
		painter->fillRect(option.rect, index.data(Qt::ForegroundRole).value<QColor>());
		if(option.state & QStyle::State_Selected){
			painter->setPen(QPen(Qt::black, 2));
			painter->drawRect(option.rect.adjusted(3, 3, -3, -3));
		}
		return;
	} else if(index.column() <= 1){
		if(option.state & QStyle::State_Selected){
			painter->fillRect(option.rect, QColor(0, 0, 0, qreal(0.5)));
		}
	}
	QStyledItemDelegate::paint(painter, option, index);
}

QWidget * ParupaintFlayerPainter::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	if(index.column() == 2){
		QLineEdit * edit = new QLineEdit(parent);
		edit->setMaxLength(64);
		this->connect(edit, &QLineEdit::textEdited, this, &ParupaintFlayerPainter::commitEdit, Qt::UniqueConnection);
		return edit;
	}
	if(index.column() == 1){
		FlayerComboBox * combo = new FlayerComboBox(parent);
		connect(combo, &FlayerComboBox::onPopupHide, this, &ParupaintFlayerPainter::commitEdit);
		return combo;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void ParupaintFlayerPainter::setEditorData(QWidget * editor, const QModelIndex & index) const
{
	QComboBox * combo = qobject_cast<QComboBox*>(editor);
	if(combo && index.column() == 1){
		foreach(QString svgMode, svgLayerModes){
			QPainter::CompositionMode mode = svgLayerModeToCompositionMode(svgMode);
			combo->addItem(compositionModeToString(mode), static_cast<int>(mode));
		}

		int mode = index.data(Qt::EditRole).toInt();
		int index = combo->findData(mode);
		if(index >= 0){
			combo->setCurrentIndex(index);
		}

		this->connect(combo, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
				this, &ParupaintFlayerPainter::commitEdit, Qt::UniqueConnection);

		combo->showPopup();
		return;
	}
	QStyledItemDelegate::setEditorData(editor, index);
}

void ParupaintFlayerPainter::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
	QComboBox * combo = qobject_cast<QComboBox*>(editor);
	if(combo && index.column() == 1){
		model->setData(index, combo->currentData(), Qt::EditRole);
		return;
	}
	QStyledItemDelegate::setModelData(editor, model, index);
}

void ParupaintFlayerPainter::commitEdit()
{
	QComboBox * combo = qobject_cast<QComboBox*>(sender());
	QLineEdit * edit = qobject_cast<QLineEdit*>(sender());
	if(combo){
		emit commitData(combo);
		emit closeEditor(combo);
	}
	if(edit){
		emit commitData(edit);
	}
}

ParupaintFlayer::ParupaintFlayer(QWidget * parent) : QTableView(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setAutoFillBackground(false);
	this->setContentsMargins(4, 4, 4, 4);

	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));

	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	this->setTabKeyNavigation(false);
	this->setShowGrid(false);

	QHeaderView * horizontal_header = this->horizontalHeader();
	QHeaderView * vertical_header = this->verticalHeader();

	if(horizontal_header) horizontal_header->hide();
	if(vertical_header) vertical_header->hide();

	if(horizontal_header)
		horizontal_header->setSectionResizeMode(QHeaderView::ResizeToContents);

	this->setItemDelegate(new ParupaintFlayerPainter);
}

void ParupaintFlayer::setCanvasModel(ParupaintCanvasModel * model)
{
	this->setModel(model);
	this->selectLayerFrame(0, 0);
}

void ParupaintFlayer::selectLayerFrame(int layer, int frame)
{
	if(layer < 0) return;
	if(layer > this->model()->rowCount()) return;

	if(!this->model()->rowCount()) return;

	// how to make this work in a better way?
	int rc = (this->model()->rowCount() > 0 ? this->model()->rowCount()-1 : 0);
	int l = (rc - layer);
	int f = (frame + 3);

	this->selectLayerFrameItem(this->model()->index(l, f));
}
void ParupaintFlayer::selectLayerFrameItem(const QModelIndex & index)
{
	QModelIndex leftmost = this->model()->index(index.row(), 0),
		    leftmost2 = this->model()->index(index.row(), 1);
	QItemSelection leftmost_selection(leftmost, leftmost2);

	QItemSelection selection(index, index);
	selection.merge(leftmost_selection, QItemSelectionModel::Select);

	this->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
	if(index.column() <= 6) {
		// snap to left
		this->scrollTo(leftmost);
	} else {
		this->scrollTo(index);
	}
}

void ParupaintFlayer::mouseMoveEvent(QMouseEvent * event)
{
	if(old_pos.isNull()) old_pos = event->pos();

	if(event->buttons() & Qt::LeftButton) return;

	if(event->buttons() & Qt::MiddleButton) {
		QPoint dif = (old_pos - event->pos());
		QScrollBar *ver = this->verticalScrollBar();
		QScrollBar *hor = this->horizontalScrollBar();
		hor->setSliderPosition(hor->sliderPosition() + int(dif.x()/3.0));
		ver->setSliderPosition(ver->sliderPosition() + int(dif.y()/3.0));
	}
	old_pos = event->pos();

	event->accept();
	QTableView::mouseMoveEvent(event);
}
void ParupaintFlayer::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::RightButton) return;

	QModelIndex index = this->indexAt(event->pos());
	if(index.column() < 3) {
		if(index.flags() & Qt::ItemIsEditable) this->edit(index);
		return;
	}

	if(index.data(Qt::ForegroundRole).value<QColor>() == Qt::transparent)
		return;

	if(event->button() == Qt::LeftButton){
		QModelIndex index = this->indexAt(event->pos());
		int rc = (this->model()->rowCount() > 0 ? this->model()->rowCount()-1 : 0);
		int l = (index.row() - rc);
		int f = (index.column() - 3);

		emit onLayerFrameSelect(l, f);
		return this->selectLayerFrameItem(index);
	}

	QTableView::mousePressEvent(event);
}

void ParupaintFlayer::mouseReleaseEvent(QMouseEvent * event)
{
	if(event && event->button() == Qt::MiddleButton){
		old_pos = QPoint();
	}
	QTableView::mouseReleaseEvent(event);
}

QSize ParupaintFlayer::minimumSizeHint() const
{
	return QSize(200, 20);
}

QSize ParupaintFlayer::sizeHint() const
{
	return QSize(800, 200);
}
