
#include "parupaintChat.h"

#include <QSizePolicy>


ParupaintChat::ParupaintChat(QWidget * parent) : ParupaintOverlayWidget(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setStyleSheet("margin:0; padding:0; border:none; background-color:transparent;");
	this->setObjectName("Chat");
	this->resize(400, 200);

	const auto lh = 25;
	
	line = new QLineEdit(this);
	line->resize(this->width(), lh);
	line->move(0, this->height() - line->height());
	line->setObjectName("ChatEntry");
	line->setFocusPolicy(Qt::ClickFocus);
	
	chat = new ParupaintChatContent(this);
	chat->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chat->setMinimumWidth(this->width());
	chat->setMaximumHeight(this->height() - lh);
}



