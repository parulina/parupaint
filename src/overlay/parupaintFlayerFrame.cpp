
#include "parupaintFlayerFrame.h"

#include <QSizePolicy>

ParupaintFlayerFrame::ParupaintFlayerFrame(QWidget * parent) : QPushButton(parent)
{
	this->setObjectName("FlayerFrame");
	this->setMaximumWidth(10);
	this->setMaximumHeight(20);
	this->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	this->setCheckable(true);
}
