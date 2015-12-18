#include "parupaintChatContent.h"

#include <QTextBrowser>
#include <QSizePolicy>
#include <QFile>
#include <QDesktopServices>
#include <QContextMenuEvent>
#include <QMenu>
#include <QDebug>
#include <QDateTime>
#include <QTextCursor>
#include <QFileDialog>

#include "../widget/parupaintScrollBar.h"

ParupaintChatContent::ParupaintChatContent(QWidget * parent) : QTextBrowser(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setStyleSheet("margin:0; padding:0; border:none; background:transparent;");
	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->document()->setDocumentMargin(0);

	QFile file(":resources/chat.css");
	file.open(QFile::ReadOnly);
	this->document()->setDefaultStyleSheet(file.readAll());
	this->setOpenExternalLinks(true);
	this->setOpenLinks(false);
	connect(this, &QTextBrowser::anchorClicked, [=](const QUrl & url){
		QDesktopServices::openUrl(url);
	});
	
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
}

void ParupaintChatContent::AddMessage(QString msg, QString who)
{
	// if it's a system message, don't escape
	// TODO make sure you can't join without a valid name
	QString str = "<span class=\"chat-message\">" + (who.isEmpty() ? msg : " " + msg.toHtmlEscaped()) + "</span>";
	if(!who.isEmpty()){
		str.prepend("<span class=\"chat-user\">" + who.toHtmlEscaped() + ":</span>");
	}
	this->AddChatMessage(str);
}

void ParupaintChatContent::AddChatMessage(QString str)
{
	// i'm proud of myself for this one!
	// ok, story time: i needed to get some padding in to .chat-entry.
	// tried settings padding, didn't work. hmm... seems like qt only
	// supports padding on table cells. well no problem, let's use cells!
	// ... but the cells inserts newlines however it wants! everywhere!
	// not good, hmm, what else can i do... QTextOption?... looking
	// through the qt sources... nope nothing...
	// IDEA! i could just insert an invisible character at the beginning
	// and end, adjust their size and give at least some vertical padding.
	// perfect!!!
	this->append(str.prepend("<div class=\"chat-entry\">\u200b").append("\u200b</div>"));
}
void ParupaintChatContent::Scroll(int x, int y)
{
	this->horizontalScrollBar()->setValue(this->horizontalScrollBar()->value() + x);
	this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() + y);
	this->update();
}

void ParupaintChatContent::focusInEvent(QFocusEvent*)
{
	emit focusIn();
}
void ParupaintChatContent::focusOutEvent(QFocusEvent*)
{
	emit focusOut();
}

void ParupaintChatContent::contextMenuEvent(QContextMenuEvent* e)
{
	auto * menu = this->createStandardContextMenu();
	QAction * action = new QAction("&Save to log file...", this);
	connect(action, &QAction::triggered, [&menu, this](bool){
		QString name = "chatlog_at_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH.mm.ss");
		this->setDocumentTitle(name);
		QString file = QFileDialog::getSaveFileName(this, "Save log file as...", "./" + this->documentTitle() + ".html", "HTML Files (*.html);; Text files (*.txt)");
		QFile file_write(file);
		if(file_write.open(QIODevice::WriteOnly)){
			QString text = (file.endsWith(".html", Qt::CaseInsensitive) ?
					this->toHtml() :
					this->toPlainText()).replace("\u200b", "");
			qDebug() << text;
			file_write.write(text.toUtf8());
			file_write.close();
		}
	});
	menu->addAction(action);
	menu->exec(e->globalPos());
}
