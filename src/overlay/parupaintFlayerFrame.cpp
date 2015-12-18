#include "parupaintFlayerFrame.h"

#include <QSizePolicy>

ParupaintFlayerFrame::ParupaintFlayerFrame(ParupaintFrame * frame, QWidget * parent) :
	QPushButton(parent),
	layer(0), frame(0)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setObjectName("FlayerFrame");
	this->setCursor(Qt::PointingHandCursor);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	this->setCheckable(true);
}

QSize ParupaintFlayerFrame::minimumSizeHint() const
{
	return QSize(60, 20);
}
