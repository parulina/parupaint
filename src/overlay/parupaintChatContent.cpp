
#include "parupaintChatContent.h"


#include <QTextBrowser>
#include <QSizePolicy>
#include <QFile>

#include "parupaintScrollBar.h"


ParupaintChatContent::ParupaintChatContent(QWidget * parent) : QScrollArea(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setStyleSheet("margin:0; padding:0; border:none; background-color:transparent;");
	this->setWidgetResizable(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	

	area = new QTextBrowser(this);
	

	QFile file(":resources/chat.css");
	file.open(QFile::ReadOnly);
	area->document()->setDefaultStyleSheet(file.readAll());
	area->setFocusPolicy(Qt::ClickFocus);
	
	area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setWidget(area);
}

void ParupaintChatContent::AddMessage(QString msg, QString who)
{
	QString str = "<span class=\"message\">" + msg + "</span>";
	if(!who.isEmpty()){
		str = "<span class=\"user\">" + who + "</span>: " + str;
	}
	this->AddChatMessage(str);
}

void ParupaintChatContent::AddChatMessage(QString str)
{
	QString html = QString("<p>%1</p>\n\r").arg(str);
	area->append(html);
}
