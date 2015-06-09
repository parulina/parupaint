
#include <QBoxLayout>
#include <QCheckBox>
#include <QSettings>

#include <QDebug>

#include "parupaintSettingsDialog.h"

ParupaintSettingsDialog::ParupaintSettingsDialog(QWidget * parent):
	ParupaintDialog(parent, "settings")
{

	this->SetSaveName("settingsDialog");

	auto * check_frameless = new QCheckBox("parupaint dialogs");
	connect(check_frameless, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("window/frameless", check_frameless->isChecked());

		this->SetFrameless(check_frameless->isChecked());
		this->show();
	});

	this->layout()->addWidget(check_frameless);

	QSettings cfg;
	check_frameless->setChecked(cfg.value("window/frameless").toBool());

}
