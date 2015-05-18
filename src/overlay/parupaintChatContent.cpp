
#include "parupaintChatContent.h"


#include <QTextBrowser>
#include <QSizePolicy>
#include <QFile>

#include "parupaintScrollBar.h"


ParupaintChatContent::ParupaintChatContent(QWidget * parent) : QScrollArea(parent)
{
	this->setStyleSheet("margin:0; padding:0; border:none; background-color:transparent;");
	this->setWidgetResizable(true);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	

	area = new QTextBrowser(this);
	
	QFile file(":resources/chat.css");
	if(file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		area->document()->setDefaultStyleSheet(file.readAll());
		file.close();
	}
	area->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->setWidget(area);
}

void ParupaintChatContent::AddMessage(QString who, QString str)
{
	AddMessage("<span class=\"user\">" + who + "</span>: <span class=\"message\">" + str + "</span>");
}

void ParupaintChatContent::AddMessage(QString str)
{
	QString html = QString("<p>%1</p>\n\r").arg(str);
	area->append(html);
}
