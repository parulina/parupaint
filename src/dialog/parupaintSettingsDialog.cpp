#include "parupaintSettingsDialog.h"

#include <QDebug>
#include <QFormLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QSettings>
#include <QHBoxLayout>
#include <QMessageBox>

#include "../widget/parupaintLineEdit.h"

ParupaintSettingsDialog::ParupaintSettingsDialog(QWidget * parent):
	ParupaintDialog(parent, "settings")
{
	this->loadGeometry("settingsDialog");
	this->setFixedWidth(480);

	ParupaintLineEdit * line_username = new ParupaintLineEdit(this, "username");
	line_username->setMaxLength(24);
	line_username->setToolTip("set the username other painters will see.");
	connect(line_username, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/username", str);
	});

	ParupaintLineEdit * line_savedir = new ParupaintLineEdit(this, "save directory");
	line_savedir->setMaxLength(24);
	line_savedir->setToolTip("set the save directory for quicksave, file browser, etc.");
	connect(line_savedir, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/directory", str);
	});

	QCheckBox * check_frameless = new QCheckBox("use in-app dialogs", this);
	check_frameless->setToolTip("toggle the modal window style.");
	connect(check_frameless, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/frameless", check_frameless->isChecked());

		this->setFrameless(check_frameless->isChecked());
	});

	QCheckBox * check_pixelgrid = new QCheckBox("use pixelgrid", this);
	check_pixelgrid->setToolTip("toggle the pixel grid.");
	connect(check_pixelgrid, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/pixelgrid", check_pixelgrid->isChecked());

		emit pixelgridChanged(check_pixelgrid->isChecked());
	});

	QPushButton * ok_button = new QPushButton("ok", this);
	connect(ok_button, &QPushButton::pressed, this, &QDialog::close);

	QPushButton * keybind_button = new QPushButton("key bindings", this);
	connect(keybind_button, &QPushButton::pressed, this, &ParupaintSettingsDialog::keyBindOpen);

	QMessageBox * confirm_reset = new QMessageBox(QMessageBox::NoIcon, "hey buddy", "sure you wanna reset your settings?",
			QMessageBox::NoButton, this);
	confirm_reset->addButton("nope", QMessageBox::RejectRole);
	confirm_reset->addButton("okay", QMessageBox::AcceptRole);
	connect(confirm_reset, &QMessageBox::accepted, this, &ParupaintSettingsDialog::confirmConfigClear);

	QPushButton * reset_button = new QPushButton("reset", this);
	reset_button->setToolTip("reset the configuation file!");
	reset_button->setFlat(true);
	connect(reset_button, &QPushButton::pressed, confirm_reset, &QMessageBox::show);

	QVBoxLayout * layout = new QVBoxLayout;

		QFormLayout * form_layout = new QFormLayout;
		form_layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
		form_layout->setFormAlignment(Qt::AlignCenter);

			form_layout->addRow("username:", line_username);
			form_layout->addRow("save directory:", line_savedir);
			form_layout->addRow(check_frameless);
			form_layout->addRow(check_pixelgrid);

		layout->addLayout(form_layout);
		QHBoxLayout * hlayout = new QHBoxLayout;
		hlayout->setAlignment(Qt::AlignBottom);
			hlayout->addWidget(reset_button, 1, Qt::AlignLeft);
			hlayout->addWidget(keybind_button);
			hlayout->addWidget(ok_button);
		layout->addLayout(hlayout);
	this->setLayout(layout);
	this->setTabOrder(reset_button, ok_button);

	QSettings cfg;
	line_username->setText(cfg.value("client/username").toString());
	line_savedir->setText(cfg.value("client/directory").toString());
	check_frameless->setChecked(cfg.value("client/frameless").toBool());
	check_pixelgrid->setChecked(cfg.value("client/pixelgrid").toBool());
}
void ParupaintSettingsDialog::confirmConfigClear()
{
	QSettings cfg;
	cfg.clear();

	emit configCleared();
}
