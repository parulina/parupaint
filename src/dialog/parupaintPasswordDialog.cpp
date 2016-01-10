#include "parupaintPasswordDialog.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "../widget/parupaintLineEdit.h"

ParupaintPasswordDialog::ParupaintPasswordDialog(QWidget * parent) :
	ParupaintDialog(parent, "server requires a password")
{
	password_line = new ParupaintLineEdit(this, "server password");

	QPushButton * knock_button = new QPushButton("knock", this);
	QPushButton * ok_button = new QPushButton("join", this);

	ok_button->setDefault(true);

	connect(ok_button, &QPushButton::pressed, this, &ParupaintPasswordDialog::checkPassword);
	connect(knock_button, &QPushButton::pressed, this, &ParupaintPasswordDialog::knockKnock);

	connect(knock_button, &QPushButton::pressed, this, &ParupaintDialog::close);
	connect(ok_button, &QPushButton::pressed, this, &ParupaintDialog::close);

	QFormLayout * flayout = new QFormLayout;
	flayout->setSizeConstraint(QLayout::SetFixedSize);

		flayout->addRow(password_line);

		QHBoxLayout * hlayout = new QHBoxLayout;
			//hlayout->addWidget(knock_button);
			hlayout->addStretch(1);
			hlayout->addWidget(ok_button);
		flayout->addRow(hlayout);

	this->setLayout(flayout);
}

void ParupaintPasswordDialog::checkPassword()
{
	const QString pw = password_line->text();
	if(pw.isEmpty()) password_line->setFocus();

	emit enterPassword(pw);
}
