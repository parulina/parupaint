#include "parupaintScrollBar.h"

#include <QDebug>
#include <QProxyStyle>
#include <QMouseEvent>

class AbsoluteBarStyle : public QProxyStyle
{
	public:
	AbsoluteBarStyle(const QString& baseStyle) : QProxyStyle(baseStyle) { }
	AbsoluteBarStyle(QStyle* baseStyle) : QProxyStyle(baseStyle) { }

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_ScrollBar_ContextMenu)
			return 0;
		if (hint == QStyle::SH_ScrollBar_LeftClickAbsolutePosition)
			return true;
		if (hint == QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl)
			return true;
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};

ParupaintScrollBar::ParupaintScrollBar(Qt::Orientation orientation, QWidget * parent, bool direction) :
	QScrollBar(orientation, parent),
	use_direction_signal(false)
{
	this->setStyle(new AbsoluteBarStyle(this->style()));
	this->setObjectName("ScrollBar");
	this->setCursor(Qt::OpenHandCursor);
	this->setUseDirection(direction);
	this->setAutoFillBackground(false);
}
// Don't move the scrollbars internally
// move them by yourself
void ParupaintScrollBar::setUseDirection(bool direction){
	use_direction_signal = direction;
}

void ParupaintScrollBar::mousePressEvent(QMouseEvent * event)
{
	old_pos = event->globalPos();

	QScrollBar::mousePressEvent(event);
}
void ParupaintScrollBar::mouseMoveEvent(QMouseEvent * event)
{
	if(this->isSliderDown()){
		QPoint dif = event->globalPos() - old_pos;
		if(!use_direction_signal){
			if(this->orientation() == Qt::Horizontal) dif.setX(0);
			if(this->orientation() == Qt::Vertical) dif.setY(0);
		}

		emit directionMove(dif);
		old_pos = event->globalPos();

		if(use_direction_signal){
			event->accept();
			return;
		}
	}
	QScrollBar::mouseMoveEvent(event);
}

