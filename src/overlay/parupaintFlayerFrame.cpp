#include "parupaintFlayerFrame.h"

#include <QSizePolicy>

ParupaintFlayerFrame::ParupaintFlayerFrame(QWidget * parent, int l, int f) :
	QPushButton(parent),
	layer(l), frame(f)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setCursor(Qt::PointingHandCursor);
	this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	this->setCheckable(true);

	connect(this, &QPushButton::clicked, this, &ParupaintFlayerFrame::onClick);
}

void ParupaintFlayerFrame::onClick()
{
	emit onLayerFrameClick(layer, frame);
}

QSize ParupaintFlayerFrame::minimumSizeHint() const
{
	return QSize(60, 20);
}
