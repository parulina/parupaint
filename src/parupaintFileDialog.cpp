#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QHBoxLayout>

#include "parupaintFileDialog.h"


ParupaintFileDialog::ParupaintFileDialog(QWidget * parent, QString filename, QString title, QString help) : 
	ParupaintDialog(parent, title, help)
{	
	label_invalid = new QLabel("...");
	label_invalid->setObjectName("ErrorLabel");
	label_invalid->hide();

	line_filename = new QLineEdit;
	line_filename->setPlaceholderText("filename");


	auto * button_browse = new QPushButton("...");
	connect(button_browse, &QPushButton::pressed, this, &ParupaintFileDialog::BrowseFiles);

	if(!filename.isEmpty()){
		line_filename->setText(filename);
		line_filename->setCursorPosition(0);
	}

	QPushButton * button_enter = new QPushButton("select");
	button_enter->setDefault(true);

	this->connect(button_enter, &QPushButton::released, this, &ParupaintFileDialog::EnterClick);
	this->connect(line_filename, &QLineEdit::textEdited, label_invalid, &QLabel::hide);

	auto * vwidget = new QWidget;
	auto * hlayout = new QHBoxLayout;
	hlayout->setMargin(0);
	hlayout->addWidget(line_filename);
	hlayout->addWidget(button_browse);
	vwidget->setLayout(hlayout);

	this->layout()->addWidget(vwidget);
	this->layout()->setAlignment(vwidget, Qt::AlignBottom);
	this->layout()->addWidget(label_invalid);
	this->layout()->setAlignment(label_invalid, Qt::AlignBottom);

	this->layout()->addWidget(button_enter);

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
