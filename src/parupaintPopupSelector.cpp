#include "parupaintPopupSelector.h"

#include <QCursor>
#include <QLabel>
#include <QLayout>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

ParupaintPopupSelector::ParupaintPopupSelector(QWidget * parent) :
	QWidget(parent)
{
	this->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
	this->setWindowModality(Qt::WindowModal);

	QHBoxLayout * layout = new QHBoxLayout;
	layout->setSpacing(1);
	layout->setMargin(1);
	this->setLayout(layout);
}

void ParupaintPopupSelector::addWidget(QWidget * widget)
{
	this->layout()->addWidget(widget);
}

void ParupaintPopupSelector::addPixmap(const QPixmap & pixmap)
{
	QLabel * label = new QLabel(this);
	label->setMargin(0);
	label->setPixmap(pixmap);
	label->resize(pixmap.size());
	this->addWidget(label);
}

void ParupaintPopupSelector::focusIndex(int index)
{
	QLayoutItem * item = this->layout()->itemAt(index);
	if(!item) return;

	QWidget * item_widget = item->widget();
	if(!item_widget) return;

	QPoint offset = item_widget->pos() + QPoint(item_widget->width()/2, item_widget->height()/2);
	this->move(QCursor::pos()-offset);
	this->show();
	this->setFocus();
}
int ParupaintPopupSelector::focusedIndex()
{
	for(int i = 0; i < this->layout()->count(); i++){
		QLayoutItem * item = this->layout()->itemAt(i);
		if(!item) continue;

		QWidget * item_widget = item->widget();
		if(!item_widget) continue;

		if(item_widget->rect().contains(item_widget->mapFromGlobal(QCursor::pos())))
			return i;
	}
	return -1;
}

void ParupaintPopupSelector::leaveEvent(QEvent * event)
{
	emit done();
}

void ParupaintPopupSelector::mousePressEvent(QMouseEvent * event)
{
	if(event->button() == Qt::LeftButton) {
		int focused_widget = this->focusedIndex();
		if(focused_widget != -1) {
			emit selectIndex(focused_widget);
			emit done();
		}
	}
}

void ParupaintPopupSelector::keyPressEvent(QKeyEvent * event)
{
	if(!event->isAutoRepeat()){
		if(event->key() != Qt::Key_Escape) {
			int focused_widget = this->focusedIndex();
			if(focused_widget != -1)
				emit selectIndex(focused_widget);
		}
		emit done();
	}
}

