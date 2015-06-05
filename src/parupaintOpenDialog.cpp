#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "parupaintOpenDialog.h"


ParupaintOpenDialog::ParupaintOpenDialog(QWidget * parent) : 
	ParupaintDialog(parent, "open file...", "enter the filename to open.")
{	
	label_invalid = new QLabel("this is not a valid filename.");
	label_invalid->setObjectName("ErrorLabel");
	label_invalid->hide();

	line_filename = new QLineEdit;
	line_filename->setPlaceholderText("filename");

	QPushButton * button_enter = new QPushButton("connect");
	button_enter->setDefault(true);

	this->connect(button_enter, &QPushButton::released, this, &ParupaintOpenDialog::EnterClick);
	this->connect(line_filename, &QLineEdit::textEdited, label_invalid, &QLabel::hide);

	this->layout()->addWidget(label_invalid);
	this->layout()->setAlignment(label_invalid, Qt::AlignBottom);

	this->layout()->addWidget(line_filename);
	this->layout()->addWidget(button_enter);

}

void ParupaintOpenDialog::EnterClick()
{
	QSettings cfg;
	QString filename = line_filename->text();
	if(!filename.isEmpty() && filename.indexOf(".") != -1){
		// if it is something and it has a dota
		cfg.setValue("net/lastopen", filename);
		emit EnterSignal(filename);
		delete this;
	} else {
		label_invalid->show();
		line_filename->setFocus();
	}
}

void ParupaintOpenDialog::showEvent(QShowEvent * )
{
	QSettings cfg;
	line_filename->setText(cfg.value("net/lastopen").toString());
	line_filename->setFocus();
}
