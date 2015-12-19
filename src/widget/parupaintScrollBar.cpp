#include "parupaintScrollBar.h"

#include <QDebug>
#include <QProxyStyle>
#include <QMouseEvent>

class AbsoluteBarStyle : public QProxyStyle
{
	int absolute_pos = 1;
	public:
	AbsoluteBarStyle(const QString& baseStyle) : QProxyStyle(baseStyle) { }
	AbsoluteBarStyle(QStyle* baseStyle) : QProxyStyle(baseStyle) { }
	void setAbsolutePos(int i) { this->absolute_pos = i; }

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_ScrollBar_ContextMenu)
			return 0;
		if (hint == QStyle::SH_ScrollBar_LeftClickAbsolutePosition)
			return absolute_pos;
		if (hint == QStyle::SH_ScrollBar_ScrollWhenPointerLeavesControl)
			return 1;
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
}
void ParupaintScrollBar::setUseDirection(bool direction){
	AbsoluteBarStyle * barstyle = static_cast<AbsoluteBarStyle*>(this->style());
	if(barstyle){
		barstyle->setAbsolutePos(direction ? 1 : 0);
	}
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

		if(use_direction_signal) return;
	}
	QScrollBar::mouseMoveEvent(event);
}

