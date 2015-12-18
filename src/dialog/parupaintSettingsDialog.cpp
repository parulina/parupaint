#include "parupaintSettingsDialog.h"

#include <QDebug>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>

#include "../widget/parupaintLineEdit.h"

ParupaintSettingsDialog::ParupaintSettingsDialog(QWidget * parent):
	ParupaintDialog(parent, "settings")
{
	this->loadGeometry("settingsDialog");
	this->setMaximumWidth(240);

	ParupaintLineEdit * username = new ParupaintLineEdit(this, "username");
	username->setMaxLength(24);
	connect(username, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/username", str);
	});

	QCheckBox * check_frameless = new QCheckBox(this);
	connect(check_frameless, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/frameless", check_frameless->isChecked());

		this->setFrameless(check_frameless->isChecked());
	});

	QCheckBox * check_pixelgrid = new QCheckBox(this);
	connect(check_pixelgrid, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/pixelgrid", check_pixelgrid->isChecked());

		emit pixelgridChanged(check_pixelgrid->isChecked());
	});

	QPushButton * ok_button = new QPushButton("ok", this);
	connect(ok_button, &QPushButton::pressed, this, &QDialog::close);

	QVBoxLayout * layout = new QVBoxLayout;

		QFormLayout * form_layout = new QFormLayout;
		form_layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
		form_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

			form_layout->addRow("username:", username);
			form_layout->addRow("native dialogs:", check_frameless);
			form_layout->addRow("use pixel grid:", check_pixelgrid);

		layout->addLayout(form_layout);
		layout->addWidget(ok_button, 1, Qt::AlignBottom);
	this->setLayout(layout);

	QSettings cfg;
	username->setText(cfg.value("client/username").toString());
	check_frameless->setChecked(cfg.value("client/frameless").toBool());
	check_pixelgrid->setChecked(cfg.value("client/pixelgrid").toBool());
}
