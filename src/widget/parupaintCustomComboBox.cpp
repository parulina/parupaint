#include "parupaintCustomComboBox.h"

#include <QStylePainter>
#include <QDebug>

ParupaintCustomComboBox::ParupaintCustomComboBox(QWidget * parent) :
	QComboBox(parent)
{

}

void ParupaintCustomComboBox::paintEvent(QPaintEvent * event)
{
	QStylePainter painter(this);
	painter.setPen(this->palette().color(QPalette::Text));

	QStyleOptionComboBox opt;
	this->initStyleOption(&opt);
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);

	painter.drawText(opt.rect, Qt::AlignCenter, opt.currentText);

	//this->QComboBox::paintEvent(event);
}
