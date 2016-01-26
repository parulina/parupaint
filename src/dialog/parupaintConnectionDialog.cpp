#include "parupaintConnectionDialog.h"

#include <QSettings>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDebug>

#include "../widget/parupaintLineEdit.h"

ParupaintConnectionDialog::ParupaintConnectionDialog(QWidget* parent) : 
	ParupaintDialog(parent, "connect...")
{
	this->loadGeometry("networkDialog");
	this->setFixedSize(280, 120);
	QSettings cfg;

	line_nickname = new ParupaintLineEdit(this, "nickname");
	line_ip = new ParupaintLineEdit(this, "host:port");

	line_nickname->setText(cfg.value("client/username").toString());
	line_ip->setText(cfg.value("client/lasthost").toString());

	QHBoxLayout * button_layout = new QHBoxLayout;
	button_layout->setMargin(0);
	button_layout->setAlignment(Qt::AlignBottom);

		QPushButton * button_connect = new QPushButton("connect", this);
		QPushButton * button_sqnya = new QPushButton("sqnya.se", this);
		QPushButton * button_disconnect = new QPushButton("disconnect", this);

		this->connect(button_disconnect, &QPushButton::pressed, this, &ParupaintConnectionDialog::onDisconnect);
		this->connect(button_sqnya,	 &QPushButton::pressed, this, &ParupaintConnectionDialog::connectClick);
		this->connect(button_connect, 	 &QPushButton::pressed, this, &ParupaintConnectionDialog::connectClick);

		this->connect(button_disconnect, &QPushButton::released, this, &QDialog::close);
		this->connect(button_sqnya,	 &QPushButton::released, this, &QDialog::close);
		this->connect(button_connect, 	 &QPushButton::released, this, &QDialog::close);

		button_connect->setDefault(true);

		button_layout->addWidget(button_connect);
		button_layout->addWidget(button_sqnya);
		button_layout->addWidget(button_disconnect);

	QVBoxLayout * layout = new QVBoxLayout;
		layout->addWidget(line_nickname);
		layout->addWidget(line_ip);
		layout->addLayout(button_layout);
	this->setLayout(layout);

	
	this->setFocusProxy(line_ip);
	//this->setFocus();
}

void ParupaintConnectionDialog::connectClick()
{
	QPushButton * button = qobject_cast<QPushButton*>(sender());
	if(line_nickname->text().isEmpty()) return line_nickname->setFocus();
	if(line_nickname->text().length() > 24) return line_nickname->setFocus();

	QSettings cfg;
	cfg.setValue("client/username", line_nickname->text());
	cfg.setValue("client/lasthost", line_ip->text());

	if(button->text() == "sqnya.se") line_ip->setText("sqnya.se");
	if(line_ip->text().isEmpty()) return line_ip->setFocus();

	emit onConnect(line_ip->text());
}
