
#include "parupaintChat.h"

#include "parupaintChatInput.h"
#include "parupaintChatContent.h"

#include <QVBoxLayout>
#include <QSizePolicy>
#include <QDebug>

ParupaintChat::ParupaintChat(QWidget * parent) : ParupaintOverlayWidget(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setObjectName("Chat");
	this->resize(400, 200);

	auto * layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);
	
	chat = new ParupaintChatContent(this);
	chat->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

	line = new ParupaintChatInput(this);
	connect(line, &QLineEdit::returnPressed, this, &ParupaintChat::returnPressed);


	layout->addWidget(chat);
	layout->addWidget(line);

	this->setFocusProxy(line);
	this->setLayout(layout);
}

void ParupaintChat::AddMessage(QString msg, QString name)
{
	chat->AddMessage(msg, name);
}

void ParupaintChat::returnPressed()
{
	QString text = line->text();
	line->setText("");
	emit Message(text);
}
