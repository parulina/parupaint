
#include "parupaintConnectionDialog.h"
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ParupaintConnectionDialog::ParupaintConnectionDialog(QWidget* parent) : ParupaintDialog(parent)
{
	this->setMinimumSize(QSize(250, 210));
	this->setFocusPolicy(Qt::NoFocus);
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	this->setWindowTitle("connect to");

	auto * layout = new QVBoxLayout(this);	
	layout->setMargin(8);

	QString help_str = QString(
		"enter your nickname and the server address here."
		"the server is in the form of <host>:<port>."
	);

	QLabel * label_help 		= new QLabel(help_str, this);
	label_help->setWordWrap(true);
	label_help->setFocusPolicy(Qt::NoFocus);
	line_nickname 			= new QLineEdit(this);
	line_nickname->setPlaceholderText("nickname");
	line_ip 			= new QLineEdit(this);
	line_ip->setPlaceholderText("<host>:<port>");
	QPushButton * button_connect	= new QPushButton("connect", this);

	layout->addWidget(label_help);
	layout->setAlignment(label_help, Qt::AlignTop);
	layout->addWidget(line_nickname);
	layout->addWidget(line_ip);
	layout->addWidget(button_connect);

	this->connect(button_connect, &QPushButton::released, this, &ParupaintConnectionDialog::ConnectClick);

	this->setLayout(layout);
	this->hide();
}

void ParupaintConnectionDialog::ConnectClick()
{
	QString name 	= line_nickname->text();
	QString ip 	= line_ip->text();
	QSettings cfg;
	if(!name.isEmpty()){
		cfg.setValue("painter/username", name);
	} else {
		line_nickname->setFocus();
	}
	if(!ip.isEmpty()){
		cfg.setValue("net/lasthost", ip);
		emit ConnectSignal(ip);
		this->hide();
	} else {
		line_ip->setFocus();
	}
}

void ParupaintConnectionDialog::showEvent(QShowEvent * )
{
	QSettings cfg;
	line_nickname->setText(cfg.value("painter/username").toString());
	line_ip->setText(cfg.value("net/lasthost").toString());

	line_ip->setFocus();

}
