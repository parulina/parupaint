
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
	connect(line, &ParupaintChatInput::pageNavigation, [=](bool up){
		int scroll_weight = 20;
		chat->Scroll(0, up ? -scroll_weight : scroll_weight);
	});
	connect(line, &ParupaintChatInput::focusIn, this, &ParupaintChat::chatInFocus);
	connect(line, &ParupaintChatInput::focusOut, this, &ParupaintChat::chatOutFocus);

	connect(chat, &ParupaintChatContent::focusIn, this, &ParupaintChat::chatInFocus);
	connect(chat, &ParupaintChatContent::focusOut, this, &ParupaintChat::chatOutFocus);


	layout->addWidget(chat);
	layout->addWidget(line);

	this->setFocusProxy(line);
	this->setLayout(layout);

	this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
}
void ParupaintChat::chatInFocus()
{
	this->setAttribute(Qt::WA_TransparentForMouseEvents, false);
}
void ParupaintChat::chatOutFocus()
{
	this->setAttribute(Qt::WA_TransparentForMouseEvents, true);
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

