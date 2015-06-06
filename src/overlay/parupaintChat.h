#ifndef PARUPAINTCHAT_H
#define PARUPAINTCHAT_H

#include "parupaintOverlayWidget.h"

class ParupaintChatInput;
class ParupaintChatContent;

class ParupaintChat : public ParupaintOverlayWidget
{
Q_OBJECT
	private:
	ParupaintChatInput *	line;
	ParupaintChatContent *	chat;

	public:
	ParupaintChat(QWidget * parent = nullptr);
	void AddMessage(QString, QString = "");

	private slots:
	void returnPressed();

	signals:
	void Message(QString);
};

#endif
