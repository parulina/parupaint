#include "parupaintNetInfo.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QDebug>

#include "../widget/parupaintLabelIcon.h"
#include "../parupaintVisualCursor.h"

// provide list of players, and a counter for spectators.

ParupaintPlayerListPainter::ParupaintPlayerListPainter(QObject * parent) :
	QStyledItemDelegate(parent)
{
	QImage image(":/resources/icons.png");
	QVector<QRgb> colors = image.colorTable();
	colors[1] = Qt::transparent;
	image.setColorTable(colors);

	icons = QPixmap::fromImage(image);
}

void ParupaintPlayerListPainter::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QStyleOptionViewItem opt = option;
	this->initStyleOption(&opt, index);

	QRect status_rect(option.rect.topLeft(), QSize(option.rect.height(), option.rect.height()));
	QRect text_rect = option.rect.adjusted(status_rect.width(), 0, 0, 0);

	if(index.data(Qt::UserRole).isValid()){
		QColor color = index.data(Qt::UserRole).value<QColor>();
		painter->fillRect(text_rect, color);
	}
	if(index.data(Qt::UserRole+1).isValid()){

		QSize icon_size(32, 32);
		int icon = index.data(Qt::UserRole + 1).toInt();
		int icon_h = icon < 0 ? icon_size.height() : 0;
		if(icon < 0) icon = -icon;

		QPoint icon_pos(icon_size.width() * icon, icon_h);

		painter->drawPixmap(status_rect, icons, QRect(icon_pos, icon_size));
	}

	painter->setPen(Qt::white);
	painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
	painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, opt.text);
}
QSize ParupaintPlayerListPainter::sizeHint(const QStyleOptionViewItem & , const QModelIndex & ) const
{
	return QSize(100, 32);
}


ParupaintPlayerList::ParupaintPlayerList(QWidget * parent) :
	QListView(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setItemDelegate(new ParupaintPlayerListPainter);
	this->setSelectionMode(QAbstractItemView::NoSelection);
	this->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

QSize ParupaintPlayerList::minimumSizeHint() const
{
	return QSize(200, 0);
}


ParupaintNetInfo::ParupaintNetInfo(QWidget * parent) :
	QFrame(parent)
{
	this->setFocusPolicy(Qt::NoFocus);

	
	num_painters = new ParupaintLabelIcon(QPixmap(":/resources/uicons.png").copy(0, 16, 16, 16), "0");
	num_spectators = new ParupaintLabelIcon(QPixmap(":/resources/uicons.png").copy(0, 0, 16, 16), "0");

	num_painters->setFixedHeight(30);
	num_spectators->setFixedHeight(30);

	list = new ParupaintPlayerList(this);

	QVBoxLayout * layout = new QVBoxLayout;
		layout->setMargin(0);
		QHBoxLayout * tophalf = new QHBoxLayout;
			tophalf->addStretch(1);
			tophalf->addWidget(num_painters, 0, Qt::AlignCenter | Qt::AlignLeft);
			tophalf->addWidget(num_spectators, 0, Qt::AlignCenter | Qt::AlignLeft);
		layout->addLayout(tophalf);
		layout->addWidget(list);
	this->setLayout(layout);

}

void ParupaintNetInfo::setCursorListModel(QAbstractListModel * model)
{
	list->setModel(model);
}

void ParupaintNetInfo::setNumPainters(int num)
{
	num_painters->setText(QString::number(num));
}
void ParupaintNetInfo::setNumSpectators(int num)
{
	num_spectators->setText(QString::number(num));
}

QSize ParupaintNetInfo::minimumSizeHint() const
{
	return QSize(200, 200);
}
