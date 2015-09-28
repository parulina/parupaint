#include <QSettings>
#include <QLabel>
#include <QFileDialog>
#include <QPushButton>
#include <QHBoxLayout>

#include "parupaintLineEdit.h"
#include "parupaintFileDialog.h"


ParupaintFileDialog::ParupaintFileDialog(QWidget * parent, QString filename, QString title, QString help) : 
	ParupaintDialog(parent, title, help)
{	
	this->SetSaveName("fileDialog");

	label_invalid = new QLabel("...");
	label_invalid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	label_invalid->setMaximumHeight(20);
	label_invalid->setObjectName("ErrorLabel");
	label_invalid->hide();

	line_filename = new ParupaintLineEdit(this, "filename");
	line_filename->setMaximumHeight(25);
	line_filename->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	auto * button_browse = new QPushButton("...");
	button_browse->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	button_browse->setMaximumHeight(25);
	connect(button_browse, &QPushButton::pressed, this, &ParupaintFileDialog::BrowseFiles);

	if(!filename.isEmpty()){
		line_filename->setText(filename);
		line_filename->setCursorPosition(0);
	}

	QPushButton * button_enter = new QPushButton("select");
	button_enter->setDefault(true);

	this->connect(button_enter, &QPushButton::released, this, &ParupaintFileDialog::EnterClick);
	this->connect(line_filename, &QLineEdit::textEdited, label_invalid, &QLabel::hide);

	auto * input_layout = new QHBoxLayout;
	input_layout->setMargin(0);
	input_layout->addWidget(line_filename);
	input_layout->addWidget(button_browse);

	auto * button_layout = new QVBoxLayout;
	button_layout->setMargin(0);
	button_layout->addWidget(label_invalid);
	button_layout->addWidget(button_enter);

	auto * layout = ((ParupaintDialogLayout*)this->layout());
	layout->addLayout(input_layout);
	layout->addLayout(button_layout);

	this->setFocusProxy(line_filename);
	this->setFocus();
}

void ParupaintFileDialog::BrowseFiles()
{
	QSettings cfg;
	QString filters = "supported files (*.png *.ora *.ppa *.tar.gz)";

	auto * file_chooser = new QFileDialog(this, "browse", ".", filters);
	file_chooser->restoreState(cfg.value("filedialog").toByteArray());
	file_chooser->show();

	connect(file_chooser, &QFileDialog::fileSelected, this, &ParupaintFileDialog::FilePick);
}
void ParupaintFileDialog::FilePick(QString file)
{
	auto * dialog = qobject_cast<QFileDialog*>(sender());
	QSettings cfg;

	cfg.setValue("filedialog", dialog->saveState());
	
	line_filename->setText(file);
	line_filename->setCursorPosition(file.length());
	this->setFocus();
}

void ParupaintFileDialog::EnterClick()
{
	QString filename = line_filename->text();
	if(!filename.isEmpty()){
		// if it is something and it has a dota
		emit EnterSignal(filename);
	} else {
		label_invalid->setText("this is not a valid filename.");
		label_invalid->show();
		line_filename->setFocus();
	}
}
