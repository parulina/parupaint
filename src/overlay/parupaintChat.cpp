#include "parupaintChat.h"

#include <QKeyEvent>

#include "parupaintChatInput.h"
#include "parupaintChatContent.h"

#include <QVBoxLayout>
#include <QSizePolicy>
#include <QDebug>

#include "../widget/parupaintScrollBar.h"

ParupaintChat::ParupaintChat(QWidget * parent) : QFrame(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->resize(400, 200);

	chat = new ParupaintChatContent(this);

	line = new ParupaintChatInput(this);
	connect(line, &QLineEdit::returnPressed, this, &ParupaintChat::returnPressed);

	auto page_up = Qt::Key_PageUp,
	     page_down = Qt::Key_PageDown,
	     exit_input = Qt::Key_Escape;

	connect(line, &ParupaintChatInput::keyPress, [=](QKeyEvent * event){

		if(event->key() == page_up || event->key() == page_down){
			int scroll_weight = (event->modifiers() & Qt::SHIFT) ? 40 : 20;
			chat->Scroll(0, (event->key() == page_up) ? -scroll_weight : scroll_weight);

			event->accept();
		} else if(event->key() == exit_input) {

			this->clearFocusAndReturn();
			event->accept();
		} else {
			emit this->onActivity();
		}

	});
	connect(line, &ParupaintChatInput::focusIn, this, &ParupaintChat::chatInFocus);
	connect(line, &ParupaintChatInput::focusOut, this, &ParupaintChat::chatOutFocus);

	connect(chat, &ParupaintChatContent::focusIn, this, &ParupaintChat::chatInFocus);
	connect(chat, &ParupaintChatContent::focusOut, this, &ParupaintChat::chatOutFocus);


	ParupaintScrollBar * vsb = new ParupaintScrollBar(Qt::Vertical, chat);
	vsb->setRange(0, 0);
	vsb->setFixedHeight(this->height() - line->height());
	connect(chat->verticalScrollBar(), &QScrollBar::rangeChanged, vsb, &QScrollBar::setRange);
	connect(vsb, &QScrollBar::valueChanged, chat->verticalScrollBar(), &QScrollBar::setValue);
	connect(chat->verticalScrollBar(), &QScrollBar::valueChanged, vsb, &QScrollBar::setValue);

	QVBoxLayout * layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(0);
		QHBoxLayout * content_layout = new QHBoxLayout;
			content_layout->addWidget(vsb, 0, Qt::AlignLeft);
			content_layout->addWidget(chat, 1, Qt::AlignBottom);
		layout->addLayout(content_layout, 1);

		layout->addWidget(line, 0);
	this->setLayout(layout);

	this->setFocusProxy(line);

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

void ParupaintChat::ClearMessages()
{
	chat->setHtml("");
}

void ParupaintChat::AddMessage(QString msg, QString name)
{
	chat->AddMessage(msg, name);
}
void ParupaintChat::setChatInputPlaceholder(QString str){
	line->setPlaceholderText(str);
}

void ParupaintChat::returnPressed()
{
	if(line->text().isEmpty()) {
		this->clearFocusAndReturn();
		return;
	}
	QString text = line->text();
	line->setText("");
	emit Message(text);
}

void ParupaintChat::clearFocusAndReturn() {
	line->clearFocus();
	this->parentWidget()->setFocus();
}
