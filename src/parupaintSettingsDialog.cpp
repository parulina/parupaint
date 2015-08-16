
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
	auto * check_pixelgrid = new QCheckBox("pixel grid");
	connect(check_pixelgrid, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/pixelgrid", check_pixelgrid->isChecked());

		emit pixelgridChanged(check_pixelgrid->isChecked());
	});

	this->layout()->addWidget(check_frameless);
	this->layout()->addWidget(check_pixelgrid);

	QSettings cfg;
	check_frameless->setChecked(cfg.value("window/frameless").toBool());
	check_pixelgrid->setChecked(cfg.value("client/pixelgrid").toBool());

}
