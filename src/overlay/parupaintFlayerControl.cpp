#include "parupaintFlayerControl.h"

#include <QHBoxLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QComboBox>

#include "../core/parupaintLayerModes.h"


ParupaintFlayerControl::ParupaintFlayerControl(QWidget * parent) :
	QFrame(parent)
{
	this->setFocusPolicy(Qt::NoFocus);

	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QHBoxLayout * layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setMargin(0);
	this->setLayout(layout);

	layer_visible = new QRadioButton(this);
	layer_visible->setFocusPolicy(Qt::NoFocus);
	layer_visible->setObjectName("FlayerVisible");
	layer_visible->setToolTip("Hide/show this layer");
	layer_visible->setFixedSize(20, 20);

	layer_mode = new QComboBox(this);
	layer_mode->setFocusPolicy(Qt::ClickFocus);
	layer_mode->setObjectName("FlayerMode");
	layer_mode->setToolTip("Set layer mode");
	layer_mode->setFixedSize(60, 20);

	foreach(QString svgMode, svgLayerModes){
		QPainter::CompositionMode mode = svgLayerModeToCompositionMode(svgMode);
		layer_mode->addItem(compositionModeToString(mode), static_cast<int>(mode));
	}

	layer_name = new QLineEdit(this);
	layer_name->setFocusPolicy(Qt::ClickFocus);
	layer_name->setObjectName("FlayerName");
	layer_name->setToolTip("Set layer name");
	layer_name->setMaxLength(64);

	connect(layer_name, &QLineEdit::textChanged, layer_name, &QLineEdit::setToolTip);

	layout->addWidget(layer_visible);
	layout->addWidget(layer_mode);
	layout->addWidget(layer_name);
	
	connect(layer_visible, &QRadioButton::clicked, this, &ParupaintFlayerControl::onLayerVisibilityChange);
	connect(layer_mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated), [this](int index){
		emit onLayerModeChange(this->layer_mode->itemData(index).toInt());
	});
	connect(layer_name, &QLineEdit::textEdited, this, &ParupaintFlayerControl::onLayerNameChange);
}

void ParupaintFlayerControl::setLayerVisible(bool visible)
{
	layer_visible->setChecked(visible);
}

void ParupaintFlayerControl::setLayerMode(int mode)
{
	int index = layer_mode->findData(mode);
	if(index >= 0){
		layer_mode->setCurrentIndex(index);
	}
}

void ParupaintFlayerControl::setLayerName(const QString & name)
{
	layer_name->setText(name);
}
