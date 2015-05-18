#ifndef PARUPAINTCHATCONTENT_H
#define PARUPAINTCHATCONTENT_H

#include <QTextBrowser>
#include <QScrollArea>

class ParupaintChatContent : public QScrollArea
{
	private:
	// list of messages and stuff
	QTextBrowser * area;

	public:
	ParupaintChatContent(QWidget * = nullptr);

	void AddMessage(QString, QString);
	void AddMessage(QString);

};

#endif
