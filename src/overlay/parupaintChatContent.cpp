
#include "parupaintChatContent.h"


#include <QTextBrowser>
#include <QSizePolicy>
#include <QFile>

#include "parupaintScrollBar.h"

ParupaintChatContent::ParupaintChatContent(QWidget * parent) : QTextBrowser(parent)
{
	this->setObjectName("ChatContent");
	this->setFocusPolicy(Qt::NoFocus);
	this->setStyleSheet("margin:0; padding:0; border:none; background-color:transparent;");
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QFile file(":resources/chat.css");
	file.open(QFile::ReadOnly);
	this->document()->setDefaultStyleSheet(file.readAll());
	this->setFocusPolicy(Qt::ClickFocus);
	this->setOpenExternalLinks(true);
	
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ParupaintChatContent::AddMessage(QString msg, QString who)
{
	// if it's a system message, don't escape
	// TODO make sure you can't join without a valid name
	QString str = "<span class=\"message\">" + (who.isEmpty() ? msg : msg.toHtmlEscaped()) + "</span>";
	if(!who.isEmpty()){
		str.prepend("<span class=\"user\"> " + who.toHtmlEscaped() + " </span>: ");
	}
	this->AddChatMessage(str);
}

void ParupaintChatContent::AddChatMessage(QString str)
{
	QString html = QString("<div class=\"chatmessage\">%1</div>\n\r").arg(str);
	this->append(html);
}
void ParupaintChatContent::Scroll(int x, int y)
{
	this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + x);
	this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + y);
	this->update();
}
