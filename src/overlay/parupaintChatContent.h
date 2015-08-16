#ifndef PARUPAINTCHATCONTENT_H
#define PARUPAINTCHATCONTENT_H

#include <QTextBrowser>

class ParupaintChatContent : public QTextBrowser
{
Q_OBJECT
	private:
	// list of messages and stuff
	void AddChatMessage(QString);
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

	public:
	ParupaintChatContent(QWidget * = nullptr);

	void AddMessage(QString, QString = "");
	void Scroll(int, int);

	signals:
	void focusIn();
	void focusOut();

};

#endif
