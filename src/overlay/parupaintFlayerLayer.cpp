#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"

#include "../core/parupaintLayer.h"

#include <QRadioButton>
#include <QComboBox>
#include <QLineEdit>
#include <QListView>
#include <QHBoxLayout>

#include <QDebug>

#include "../widget/parupaintCustomComboBox.h"

ParupaintFlayerLayer::ParupaintFlayerLayer(int l, QWidget * parent, bool visible, int mode, const QString & name) :
	QWidget(parent),
	layer(l)
{
	this->setFocusPolicy(Qt::NoFocus);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	layout->setAlignment(Qt::AlignLeft);
	this->setLayout(layout);


	layer_visible = new QRadioButton(this);
	layer_visible->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	layer_visible->setFixedWidth(26);
	layer_visible->setToolTip("hide/show layer");
	layer_visible->setFocusPolicy(Qt::NoFocus);

	layer_mode = new ParupaintCustomComboBox(this);
	layer_mode->setView(new QListView());
	layer_mode->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	layer_mode->setFixedWidth(90);
	layer_mode->setToolTip("set layer mode");
	layer_mode->setFocusPolicy(Qt::NoFocus);

	layer_mode->setEditable(false);

	layer_mode->addItem("Normal", 		textModeToEnum("svg:src-over"));
	layer_mode->addItem("Multiply", 	textModeToEnum("svg:multiply"));
	layer_mode->addItem("Screen", 		textModeToEnum("svg:screen"));
	layer_mode->addItem("Overlay", 		textModeToEnum("svg:overlay"));
	layer_mode->addItem("Darken", 		textModeToEnum("svg:darken"));
	layer_mode->addItem("Lighten", 		textModeToEnum("svg:lighten"));
	layer_mode->addItem("Color dodge", 	textModeToEnum("svg:color-dodge"));
	layer_mode->addItem("Color burn", 	textModeToEnum("svg:color-burn"));
	layer_mode->addItem("Hard light", 	textModeToEnum("svg:hard-light"));
	layer_mode->addItem("Soft light", 	textModeToEnum("svg:soft-light"));
	layer_mode->addItem("Difference", 	textModeToEnum("svg:difference"));
	layer_mode->addItem("Color", 		textModeToEnum("svg:color"));
	layer_mode->addItem("Luminosity", 	textModeToEnum("svg:luminosity"));
	layer_mode->addItem("Hue", 		textModeToEnum("svg:hue"));
	layer_mode->addItem("Saturation", 	textModeToEnum("svg:saturation"));
	layer_mode->addItem("Plus", 		textModeToEnum("svg:plus"));
	layer_mode->addItem("Dest in",		textModeToEnum("svg:dst-in"));
	layer_mode->addItem("Dest out", 	textModeToEnum("svg:dst-out"));
	layer_mode->addItem("Source ontop", 	textModeToEnum("svg:src-atop"));
	layer_mode->addItem("Dest ontop", 	textModeToEnum("svg:dst-atop"));


	layer_name = new QLineEdit(this);
	layer_name->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
	layer_name->setFixedWidth(120);
	layer_name->setToolTip("set layer name");
	layer_name->setFocusPolicy(Qt::ClickFocus);
	layer_name->setAlignment(Qt::AlignLeft);
	layer_name->setMaxLength(10);

	layer_visible->setObjectName("FlayerLayerVisible");
	layer_mode->setObjectName("FlayerLayerMode");
	layer_name->setObjectName("FlayerLayerLabel");

	this->setLayerVisible(visible);
	this->setLayerMode(mode);
	this->setLayerName(name);

	layout->addWidget(layer_visible, 0, Qt::AlignHCenter);
	layout->addWidget(layer_mode);
	layout->addWidget(layer_name);

	// important to use only the UI related signals so that it doesn't
	// infinite loop when setting programmatically
	connect(layer_visible, &QRadioButton::clicked, this, &ParupaintFlayerLayer::layerToggleVisibility);
	connect(layer_name, &QLineEdit::textEdited, this, &ParupaintFlayerLayer::layerChangeName);
	connect(layer_mode, static_cast<void(QComboBox::*)(int)>(&QComboBox::activated),
	        this, &ParupaintFlayerLayer::layerChangeMode);
}

void ParupaintFlayerLayer::layerToggleVisibility(bool toggle)
{
	emit visibleChanged(layer, toggle);
}
void ParupaintFlayerLayer::layerChangeName(const QString & name)
{
	emit nameChanged(layer, name);
}
void ParupaintFlayerLayer::layerChangeMode(int mode)
{
	QComboBox * modes = qobject_cast<QComboBox*>(sender());
	if(modes){
		int rm = modes->itemData(mode).toInt();
		emit modeChanged(layer, rm);
	}
}

void ParupaintFlayerLayer::setLayerVisible(bool visible)
{
	if(visible == layer_visible->isChecked()) return;
	layer_visible->setChecked(visible);
}
void ParupaintFlayerLayer::setLayerMode(int mode)
{
	if(layer_mode->currentIndex() == layer_mode->findData(mode)) return;
	layer_mode->setCurrentIndex(layer_mode->findData(mode));
}
void ParupaintFlayerLayer::setLayerName(const QString & name)
{
	if(layer_name->text() == name) return;
	layer_name->setText(name);
}


void ParupaintFlayerLayer::addFrame(ParupaintFlayerFrame * frame)
{
	QHBoxLayout * layout = qobject_cast<QHBoxLayout*>(this->layout());
	layout->addWidget(frame, 0);
}
ParupaintFlayerFrame * ParupaintFlayerLayer::frameAt(int i)
{
	QHBoxLayout * layout = qobject_cast<QHBoxLayout*>(this->layout());

	QLayoutItem * item = layout->itemAt(i + 3);
	if(!item) return nullptr;

	return qobject_cast<ParupaintFlayerFrame*>(item->widget());
}

void ParupaintFlayerLayer::selectFrame(int f)
{
	ParupaintFlayerFrame * ff;
	int frame = 0;
	while((ff = this->frameAt(frame))){
		if(frame == f){
			ff->setChecked(true);
		} else {
			ff->setChecked(false);
		}
		frame++;
	}
}
