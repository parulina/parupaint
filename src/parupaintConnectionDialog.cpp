
#include "parupaintConnectionDialog.h"
#include <QSettings>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

ParupaintConnectionDialog::ParupaintConnectionDialog(QWidget* parent) : 
	ParupaintDialog(parent, "connect...", "enter your nickname and the server address here. the server is in the form of <host>:<port>.")
{
	this->setMinimumSize(280, 160);
	QSettings cfg;

	line_nickname = new QLineEdit();
	line_nickname->setPlaceholderText("nickname");
	line_nickname->setText(cfg.value("painter/username").toString());

	line_ip = new QLineEdit();
	line_ip->setPlaceholderText("<host>:<port>");
	line_ip->setText(cfg.value("net/lasthost").toString());

	auto * button_layout = new QHBoxLayout;
	button_layout->setMargin(0);

	auto * button_connect = new QPushButton("connect");
	button_connect->setDefault(true);
	this->connect(button_connect, &QPushButton::released, this, &ParupaintConnectionDialog::ConnectClick);
	
	auto * button_disconnect = new QPushButton("disconnect");
	this->connect(button_disconnect, &QPushButton::pressed, this, &ParupaintConnectionDialog::DisconnectSignal);

	button_layout->addWidget(button_connect);
	button_layout->addWidget(button_disconnect);

	auto * layout = ((QVBoxLayout*) this->layout());
	this->layout()->addWidget(line_nickname);
	this->layout()->addWidget(line_ip);
	layout->addLayout(button_layout);

	
	this->setFocusProxy(line_ip);
	this->setFocus();
}

void ParupaintConnectionDialog::ConnectClick()
{
	QString name 	= line_nickname->text();
	QString ip 	= line_ip->text();
	QSettings cfg;

	if(!name.isEmpty()){
		cfg.setValue("painter/username", name);
	} else {
		return line_nickname->setFocus();
	}
	if(!ip.isEmpty()){
		cfg.setValue("net/lasthost", ip);
		emit ConnectSignal(ip);
	} else {
		return line_ip->setFocus();
	}
}
