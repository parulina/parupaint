#ifndef PARUPAINTCHATCONTENT_H
#define PARUPAINTCHATCONTENT_H

#include <QTextBrowser>

class ParupaintChatContent : public QTextBrowser
{
	private:
	// list of messages and stuff
	void AddChatMessage(QString);

	public:
	ParupaintChatContent(QWidget * = nullptr);

	void AddMessage(QString, QString = "");
	void Scroll(int, int);

};

#endif
