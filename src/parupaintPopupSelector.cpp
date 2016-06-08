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

	this->show();
	this->setFocus();

	QPoint offset = item_widget->pos() + QPoint(item_widget->width()/2, item_widget->height()/2);
	this->move(QCursor::pos()-offset);

	item_widget->update();
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
	if((event->key() != Qt::Key_Shift && event->key() != Qt::Key_Control) && !event->isAutoRepeat()){

		int focused_widget = this->focusedIndex();
		if(event->key() == Qt::Key_Left || event->key() == Qt::Key_Backtab) focused_widget--;
		if(event->key() == Qt::Key_Right || event->key() == Qt::Key_Tab) focused_widget++;

		if(focused_widget != this->focusedIndex()){
			if(focused_widget < 0) focused_widget = this->layout()->count()-1;
			if(focused_widget > this->layout()->count()-1) focused_widget = 0;
			this->focusIndex(focused_widget);
			return;
		}

		if(event->key() >= Qt::Key_1 && event->key() <= Qt::Key_9){
			int slot = event->key() - Qt::Key_1;
			if(slot < this->layout()->count()){
				emit selectIndex(slot);
			}

		} else if(event->key() != Qt::Key_Escape) {
			int focused_widget = this->focusedIndex();
			if(focused_widget != -1)
				emit selectIndex(focused_widget);
		}
		emit done();
	}
}

