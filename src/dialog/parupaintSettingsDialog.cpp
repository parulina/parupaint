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

	ParupaintLineEdit * line_username = new ParupaintLineEdit(this, "Username");
	line_username->setMaxLength(24);
	line_username->setToolTip("Set the username other painters will see.");
	connect(line_username, &QLineEdit::textEdited, this, &ParupaintSettingsDialog::nameChanged);
	connect(line_username, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/username", str);
	});

	ParupaintLineEdit * line_sessionpw = new ParupaintLineEdit(this, "Random password (enter \"none\" to disable password)");
	line_sessionpw->setToolTip("Set the session password. (Note: requires restart)");
	line_sessionpw->setMaxLength(64);
	connect(line_sessionpw, &QLineEdit::textEdited, this, &ParupaintSettingsDialog::sessionPasswordChanged);
	connect(line_sessionpw, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/sessionpassword", str);
	});

	ParupaintLineEdit * line_savedir = new ParupaintLineEdit(this, "Save directory");
	line_savedir->setToolTip("Set the save directory for quicksave, file browser, etc.");
	connect(line_savedir, &QLineEdit::textEdited, [](const QString & str){
		QSettings cfg;
		cfg.setValue("client/directory", str);
	});

	QCheckBox * check_frameless = new QCheckBox("Use in-app dialogs", this);
	check_frameless->setToolTip("Toggle the modal window style.");
	connect(check_frameless, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/frameless", check_frameless->isChecked());

		this->setFrameless(check_frameless->isChecked());
	});

	QCheckBox * check_pixelgrid = new QCheckBox("Use pixelgrid", this);
	check_pixelgrid->setToolTip("Toggle the pixel grid.");
	connect(check_pixelgrid, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/pixelgrid", check_pixelgrid->isChecked());

		emit pixelgridChanged(check_pixelgrid->isChecked());
	});

	QCheckBox * check_cursor = new QCheckBox("Show cursor pointer", this);
	check_cursor->setToolTip("Toggle the visiblity of the canvas pointer.");
	connect(check_cursor, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/cursor", check_cursor->isChecked());

		emit cursorModeChanged(check_cursor->isChecked());
	});

	QCheckBox * check_viewport = new QCheckBox("Fast viewport", this);
	check_viewport->setToolTip("Toggle the 'full' update mode of the viewport. Less rendering artifacts, but may fix some problems.");
	connect(check_viewport, &QCheckBox::stateChanged, [=](int){
		QSettings cfg;
		cfg.setValue("client/fastviewport", check_viewport->isChecked());

		emit viewportFastUpdateChanged(check_viewport->isChecked());
	});

	QPushButton * ok_button = new QPushButton("Done", this);
	connect(ok_button, &QPushButton::pressed, this, &QDialog::close);

	QPushButton * keybind_button = new QPushButton("Hotkeys", this);
	connect(keybind_button, &QPushButton::pressed, this, &ParupaintSettingsDialog::keyBindOpen);

	QMessageBox * confirm_reset = new QMessageBox(QMessageBox::NoIcon, "Hey buddy", "Sure you wanna reset your settings?",
			QMessageBox::NoButton, this);
	confirm_reset->addButton("Nope", QMessageBox::RejectRole);
	confirm_reset->addButton("Okay", QMessageBox::AcceptRole);
	connect(confirm_reset, &QMessageBox::accepted, this, &ParupaintSettingsDialog::confirmConfigClear);

	QPushButton * reset_button = new QPushButton("Reset", this);
	reset_button->setToolTip("Reset the configuation file!");
	reset_button->setFlat(true);
	connect(reset_button, &QPushButton::pressed, confirm_reset, &QMessageBox::show);

	QVBoxLayout * layout = new QVBoxLayout;

		QFormLayout * form_layout = new QFormLayout;
		form_layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
		form_layout->setFormAlignment(Qt::AlignCenter);

			form_layout->addRow("Username:", line_username);
			form_layout->addRow("Your session password:", line_sessionpw);
			form_layout->addRow("Save directory:", line_savedir);
			form_layout->addRow(check_frameless);
			form_layout->addRow(check_pixelgrid);
			form_layout->addRow(check_cursor);
			form_layout->addRow(check_viewport);

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
	line_sessionpw->setText(cfg.value("client/sessionpassword").toString());
	line_savedir->setText(cfg.value("client/directory").toString());
	check_frameless->setChecked(cfg.value("client/frameless", true).toBool());
	check_pixelgrid->setChecked(cfg.value("client/pixelgrid", true).toBool());
	check_cursor->setChecked(cfg.value("client/cursor", true).toBool());
	check_viewport->setChecked(cfg.value("client/fastviewport", false).toBool());
}
void ParupaintSettingsDialog::confirmConfigClear()
{
	QSettings cfg;
	cfg.clear();

	emit configCleared();
}
