
#include "parupaintChat.h"

#include <QSizePolicy>


ParupaintChat::ParupaintChat(QWidget * parent) : ParupaintOverlayWidget(parent)
{
	setStyleSheet("margin:0; padding:0; border:none; background-color:transparent;");
	setObjectName("Chat");
	resize(400, 200);

	const auto lh = 25;
	
	// TODO auto height and stuff...

	line = new QLineEdit(this);
	line->resize(this->width(), lh);
	line->move(0, this->height() - line->height());
	// hmm.. that didn't work...
	line->setStyleSheet("background-color:black;");
	
	chat = new ParupaintChatContent(this);
	chat->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chat->setMinimumWidth(this->width());
	chat->setMaximumHeight(this->height() - lh);
}



