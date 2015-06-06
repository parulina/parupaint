#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

#include "parupaintFileDialog.h"


ParupaintFileDialog::ParupaintFileDialog(QWidget * parent, QString filename, QString title, QString help) : 
	ParupaintDialog(parent, title, help)
{	
	label_invalid = new QLabel("this is not a valid filename.");
	label_invalid->setObjectName("ErrorLabel");
	label_invalid->hide();

	line_filename = new QLineEdit;
	line_filename->setPlaceholderText("filename");

	if(!filename.isEmpty()){
		line_filename->setText(filename);
		line_filename->setCursorPosition(0);
	}

	QPushButton * button_enter = new QPushButton("connect");
	button_enter->setDefault(true);

	this->connect(button_enter, &QPushButton::released, this, &ParupaintFileDialog::EnterClick);
	this->connect(line_filename, &QLineEdit::textEdited, label_invalid, &QLabel::hide);

	this->layout()->addWidget(label_invalid);
	this->layout()->setAlignment(label_invalid, Qt::AlignBottom);

	this->layout()->addWidget(line_filename);
	this->layout()->addWidget(button_enter);

	this->setFocusProxy(line_filename);
	this->setFocus();
}

void ParupaintFileDialog::EnterClick()
{
	QString filename = line_filename->text();
	if(!filename.isEmpty() && filename.indexOf(".") != -1){
		// if it is something and it has a dota
		emit EnterSignal(filename);
		delete this;
	} else {
		label_invalid->show();
		line_filename->setFocus();
	}
}
