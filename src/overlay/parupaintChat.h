#ifndef PARUPAINTCHAT_H
#define PARUPAINTCHAT_H

#include <QFrame>

class ParupaintChatInput;
class ParupaintChatContent;

class ParupaintChat : public QFrame
{
Q_OBJECT
	private:
	ParupaintChatInput *	line;
	ParupaintChatContent *	chat;
	void clearFocusAndReturn();

	public:
	ParupaintChat(QWidget * parent = nullptr);
	void ClearMessages();
	void AddMessage(QString, QString = "");
	void setChatInputPlaceholder(QString);

	private slots:
	void returnPressed();
	void chatInFocus();
	void chatOutFocus();

	signals:
	void onActivity();
	void Message(QString);
};

#endif
