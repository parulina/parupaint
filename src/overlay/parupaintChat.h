#ifndef PARUPAINTCHAT_H
#define PARUPAINTCHAT_H

#include "parupaintOverlayWidget.h"
#include "parupaintChatContent.h"

#include <QLineEdit>

class ParupaintChat : public ParupaintOverlayWidget
{
	private:
	QLineEdit *	line;
	ParupaintChatContent *	chat;


	public:
	ParupaintChat(QWidget * parent = nullptr);

	private:
};

#endif
