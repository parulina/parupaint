#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"

#include <QRadioButton>
#include <QLineEdit>
#include <QHBoxLayout>

ParupaintFlayerLayer::ParupaintFlayerLayer(QWidget * parent) : QWidget(parent),
	firstchildren_count(-1)
{
	this->setFocusPolicy(Qt::NoFocus);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignLeft);
	this->setLayout(layout);

	layer_visible = new QRadioButton(this);
	layer_visible->setFocusPolicy(Qt::NoFocus);
	layer_visible->setObjectName("FlayerLayerVisible");
	layer_visible->setToolTip("hide/show layer");
	layer_visible->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	layer_visible->setFixedWidth(20);

	layer_name = new QLineEdit(this);
	layer_name->setFocusPolicy(Qt::ClickFocus);
	layer_name->setObjectName("FlayerLayerLabel");
	layer_name->setToolTip("set layer name");
	layer_name->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	layer_name->setFixedWidth(120);
	layer_name->setText("layer");
	layer_name->setAlignment(Qt::AlignLeft);
	layer_name->setMaxLength(10);

	//layer_name->setText(QString(" layer %1 ").arg(l + 1));

	layout->addWidget(layer_visible, 0, Qt::AlignHCenter);
	layout->addWidget(layer_name);
}
void ParupaintFlayerLayer::setLayerVisible(bool visible)
{
	layer_visible->setChecked(visible);
}
void ParupaintFlayerLayer::setName(const QString & name)
{
	layer_name->setText(name);
}
void ParupaintFlayerLayer::addFrame(ParupaintFlayerFrame * frame)
{
	QHBoxLayout * layout = qobject_cast<QHBoxLayout*>(this->layout());
	int stretch = 0;
	if(firstchildren_count == -1){
		firstchildren_count = layout->count();
		stretch = 1;
	} else {
		layout->setStretch(firstchildren_count, 0);
	}
	layout->addWidget(frame, stretch);
}
ParupaintFlayerFrame * ParupaintFlayerLayer::frameAt(int i)
{
	QHBoxLayout * layout = qobject_cast<QHBoxLayout*>(this->layout());
	if(firstchildren_count == -1) return nullptr;
	QLayoutItem * item = layout->itemAt(i + firstchildren_count);
	if(!item) return nullptr;

	return qobject_cast<ParupaintFlayerFrame*>(item->widget());
}

