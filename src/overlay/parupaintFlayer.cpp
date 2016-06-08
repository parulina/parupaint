#include "parupaintFlayer.h"

#include <QPainter>
#include <QHeaderView>
#include <QMouseEvent>
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QLabel>
#include <QEvent>
#include <QDebug>

#include "../parupaintVisualCanvas.h"
#include "../widget/parupaintScrollBar.h"
#include "../core/parupaintLayerModes.h"

#include "parupaintFlayerControl.h"

ParupaintFlayerPainter::ParupaintFlayerPainter(QObject * parent) :
	QStyledItemDelegate(parent)
{
}
void ParupaintFlayerPainter::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItem opt(option);
	opt.state &= ~QStyle::State_Selected;
	QStyledItemDelegate::paint(painter, opt, index);

	if(option.state & QStyle::State_Selected){
		painter->save();

		painter->setPen(QPen(Qt::white, 1));
		painter->setCompositionMode(QPainter::CompositionMode_Difference);
		painter->drawRect(option.rect.adjusted(2, 2, -2, -2));

		painter->restore();
	}
}

ParupaintFixedViewport::ParupaintFixedViewport(QWidget * parent) : 
	QAbstractScrollArea(parent)
{
}

bool ParupaintFixedViewport::viewportEvent(QEvent * event)
{
	// scrollContentsBy doesn't seem to get called. (QT bug?)
	// I'm using this instead
	if(event->type() == QEvent::Move){
		QMoveEvent * move_event = static_cast<QMoveEvent*>(event);
		const QPoint dif = move_event->pos() - move_event->oldPos();
		emit contentsScrolledBy(dif.x(), dif.y());
	}
	return this->QAbstractScrollArea::viewportEvent(event);
}

ParupaintFlayerControlHeader::ParupaintFlayerControlHeader(QWidget * parent) :
	QHeaderView(Qt::Vertical, parent)
{
	this->setSectionResizeMode(QHeaderView::Fixed);
	this->setDefaultSectionSize(20);

	this->setMinimumWidth(160);
	this->setAutoFillBackground(false);

	ParupaintFixedViewport * viewport = new ParupaintFixedViewport(this);
	this->setViewport(viewport);
	connect(viewport, &ParupaintFixedViewport::contentsScrolledBy, this, &ParupaintFlayerControlHeader::fixControlPositions);
}

void ParupaintFlayerControlHeader::fixControlPositions()
{
	for(int i = 0; i < layer_controls.size(); i++){
		ParupaintFlayerControl * control = layer_controls.at(i);
		control->setGeometry(0, this->sectionViewportPosition(i), this->width(), this->sectionSize(i));
	}
}

void ParupaintFlayerControlHeader::paintSection(QPainter *painter, const QRect & rect, int logicalIndex) const
{
	if(this->currentIndex().row() == logicalIndex){
		const QColor highlight_color("#75653A");
		painter->fillRect(rect, highlight_color);
	}
	this->QHeaderView::paintSection(painter, rect, logicalIndex);
}
void ParupaintFlayerControlHeader::setModel(QAbstractItemModel * model)
{
	this->QHeaderView::setModel(model);
	connect(model, &QAbstractItemModel::layoutChanged, this, &ParupaintFlayerControlHeader::layoutChange);
	connect(model, &QAbstractItemModel::headerDataChanged, this, &ParupaintFlayerControlHeader::headerDataChange);
}

void ParupaintFlayerControlHeader::layoutChange(const QList<QPersistentModelIndex> & parents)
{
	qDeleteAll(layer_controls);
	layer_controls.clear();

	for(int i = 0; i < this->count(); i++){
		ParupaintFlayerControl * control = new ParupaintFlayerControl(this);

		connect(control, &ParupaintFlayerControl::onLayerVisibilityChange, [this, i](bool visible){
			this->model()->setHeaderData(i, this->orientation(), visible, ParupaintCanvasModel::LayerVisibleRole);
		});
		connect(control, &ParupaintFlayerControl::onLayerModeChange, [this, i](int mode){
			this->model()->setHeaderData(i, this->orientation(), mode, ParupaintCanvasModel::LayerModeRole);
		});
		connect(control, &ParupaintFlayerControl::onLayerNameChange, [this, i](const QString & name){
			this->model()->setHeaderData(i, this->orientation(), name, ParupaintCanvasModel::LayerNameRole);
		});

		control->setLayerVisible(this->model()->headerData(i, this->orientation(), ParupaintCanvasModel::LayerVisibleRole).toBool());
		control->setLayerMode(this->model()->headerData(i, this->orientation(), ParupaintCanvasModel::LayerModeRole).toInt());
		control->setLayerName(this->model()->headerData(i, this->orientation(), ParupaintCanvasModel::LayerNameRole).toString());
		this->setIndexWidget(this->model()->index(i, -1), control);

		control->show();

		layer_controls << control;
	}
	this->fixControlPositions();
}

void ParupaintFlayerControlHeader::headerDataChange(Qt::Orientation orientation, int first, int last)
{
	ParupaintFlayerControl * control = layer_controls.at(first);
	if(control){
		control->setLayerVisible(this->model()->headerData(first, this->orientation(), ParupaintCanvasModel::LayerVisibleRole).toBool());
		control->setLayerMode(this->model()->headerData(first, this->orientation(), ParupaintCanvasModel::LayerModeRole).toInt());
		control->setLayerName(this->model()->headerData(first, this->orientation(), ParupaintCanvasModel::LayerNameRole).toString());
	}
}


ParupaintFlayer::ParupaintFlayer(QWidget * parent) : QTableView(parent)
{
	this->setItemDelegate(new ParupaintFlayerPainter);

	this->setFocusPolicy(Qt::NoFocus);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setAutoFillBackground(false);
	this->setContentsMargins(4, 4, 4, 4);

	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));

	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setDragDropMode(QAbstractItemView::InternalMove);
	this->setDragEnabled(true);
	this->setDropIndicatorShown(true);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	this->setTabKeyNavigation(false);
	this->setShowGrid(false);

	QHeaderView * horizontal_header = this->horizontalHeader();
	horizontal_header->setSectionResizeMode(QHeaderView::Fixed);
	horizontal_header->setContentsMargins(0, 0, 0, 0);
	horizontal_header->setFixedHeight(10);
	horizontal_header->setHighlightSections(false);

	this->setVerticalHeader(new ParupaintFlayerControlHeader(this));
}

// FIXME a way to sort the rows in reverse order would be wonderful
// but i can't figure out a way to do that. help?
int ParupaintFlayer::modelLayer(int layer)
{
	int rc = (this->model()->rowCount() > 0 ? this->model()->rowCount()-1 : 0);
	return rc - layer;
}

void ParupaintFlayer::itemActivated(const QModelIndex & current, const QModelIndex & previous)
{
	int l = this->modelLayer(current.row());
	int f = (current.column());
	emit onLayerFrameSelect(l, f);
}

void ParupaintFlayer::setCanvasModel(ParupaintCanvasModel * model)
{
	this->setModel(model);
	this->selectLayerFrame(0, 0);

	connect(this->selectionModel(), &QItemSelectionModel::currentChanged, this, &ParupaintFlayer::itemActivated);
}

void ParupaintFlayer::selectLayerFrame(int layer, int frame)
{
	if(layer < 0) return;
	if(layer > this->model()->rowCount()) return;
	if(!this->model()->rowCount()) return;

	layer = this->modelLayer(layer);
	this->selectLayerFrameItem(this->model()->index(layer, frame));
}

void ParupaintFlayer::selectLayerFrameItem(const QModelIndex & index)
{
	this->selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);

	int tc = this->model()->columnCount(index)-1;
	int tt = 2;

	// sticky the view to the left or right based on where we are
	if(index.column() <= tt) {
		this->scrollTo(this->model()->index(index.row(), 0));

	} else if(index.column() >= tc-tt){
		this->scrollTo(this->model()->index(index.row(), tc));

	} else {
		this->scrollTo(index);
	}
}

void ParupaintFlayer::mouseMoveEvent(QMouseEvent * event)
{
	if(old_pos.isNull()) old_pos = event->pos();

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
	if(event->button() != Qt::RightButton){
		return;
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

void ParupaintFlayer::resizeEvent(QResizeEvent * event)
{
	qDebug() << event;

	bool small_view = (event->size().height() <= 20);
	this->setHorizontalScrollBarPolicy(small_view ? Qt::ScrollBarAlwaysOff : Qt::ScrollBarAlwaysOn);

	this->QTableView::resizeEvent(event);
}

QSize ParupaintFlayer::minimumSizeHint() const
{
	return QSize(200, 20);
}

QSize ParupaintFlayer::sizeHint() const
{
	return QSize(800, 200);
}
