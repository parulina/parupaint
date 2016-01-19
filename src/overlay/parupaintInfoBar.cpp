#include "parupaintInfoBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QFile> // stylesheet
#include <QDebug>

#include "../widget/parupaintScrollBar.h"

QString stylesheet = "";

ParupaintInfoBarText::ParupaintInfoBarText(QWidget * parent) : QTextBrowser(parent)
{
	this->setAutoFillBackground(false);
	this->document()->setDocumentMargin(2);
	this->document()->setDefaultStyleSheet(stylesheet);
	this->setFocusPolicy(Qt::ClickFocus);
	this->setOpenLinks(true);
	this->setOpenExternalLinks(true);
	this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));
	this->setHtml(QStringLiteral(
	 "<p class=\"about\">"
		"<h3>PARUPAINT</h3>"
		"<a href=\"http://parupaint.sqnya.se\">official homepage</a>&nbsp;&nbsp;&nbsp;<a href=\"http://github.com/parulina/parupaint\">github</a><br/>"
		"Welcome to my painting program, parupaint. The program is designed to be quick and light, while still being a nice drawing application. You can read a summary of what this program does on the homepage. Thank you for downloading and using this!<br/>"
		"<span class=\"notice\">Please note that this program is currently in alpha and is constantly adding/removing features.</span>"
	 "</p>"
	));
}
QSize ParupaintInfoBarText::minimumSizeHint() const
{
	return QSize(600, 0);
}

ParupaintInfoBarTutorial::ParupaintInfoBarTutorial(QWidget * parent) : QTextBrowser(parent)
{
	this->setAutoFillBackground(false);
	this->document()->setDocumentMargin(2);
	this->document()->setDefaultStyleSheet(stylesheet);
	this->setFocusPolicy(Qt::ClickFocus);
	this->setOpenLinks(true);
	this->setOpenExternalLinks(true);
	this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));
	this->setFixedWidth(400);
	this->setHtml(QStringLiteral(
	 "<p class=\"mini-tutorial\">"
		 "<h3>QUICK START (DEFAULT KEYS)</h3>"
		 "<a href=\"http://parupaint.sqnya.se/tutorial.html\">full tutorial</a><br/>"
		 "Press Ctrl+K to quicksave, and Ctrl+M for settings.<br/>"
		 "<span class=\"notice\">The settings has a full key reference.</span><br/>"
		 "Hold Space to move canvas, and Ctrl+Space to zoom. Hold Shift+Space to change brush size. Press W for halftones, Q for fill tool, T to test fill.<br/>"
		 "<u>Press Shift+Tab to hide this!</u>"
	 "</p>"
	));
}
QSize ParupaintInfoBarTutorial::minimumSizeHint() const
{
	return QSize(400, 0);
}

ParupaintInfoBarStatus::ParupaintInfoBarStatus(QWidget * parent) : QTextBrowser(parent)
{
	this->setAutoFillBackground(false);
	this->document()->setDocumentMargin(10);
	this->document()->setDefaultStyleSheet(stylesheet);
	this->setFixedHeight(30);
	this->setOpenLinks(false);
	this->setContentsMargins(0, 0, 0, 0);

	this->setFocusPolicy(Qt::ClickFocus); // NoFocus
	this->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
void ParupaintInfoBarStatus::updateTitle()
{
	this->setHtml(QString(
		"<p class=\"titletext\">"
			"<a href=\"#f1_notice\">(help: F1)</a>  "
			"<a href=\"#connected\">connected to " + (connected_to == "localhost" ? "yourself" : connected_to) + "</a>  "
			"<a href=\"#dimensions\">&lt;" + dimensions + "&gt;</a>  "
			"<span href=\"#layerframe\">{" + layerframe + "}</span>"
		"</p>"
	));
}
void ParupaintInfoBarStatus::setConnectedTo(const QString & con)
{
	connected_to = con;
	this->updateTitle();
}
void ParupaintInfoBarStatus::setDimensions(const QSize & size)
{
	dimensions = QString("%1 x %2").arg(QString::number(size.width()), QString::number(size.height()));
	this->updateTitle();
}
void ParupaintInfoBarStatus::setLayerFrame(int layer, int frame)
{
	layerframe = QString("%1 : %2").arg(QString::number(layer + 1), QString::number(frame + 1));
	this->updateTitle();
}

// InfoBar
ParupaintInfoBar::ParupaintInfoBar(QWidget * parent) : QWidget(parent)
{
	this->setFocusPolicy(Qt::NoFocus);

	QFile file(":/resources/chat.css");
	file.open(QFile::ReadOnly);
	stylesheet = file.readAll();

	QVBoxLayout * box = new QVBoxLayout;
	box->setMargin(0);
	box->setSpacing(0);
	box->setContentsMargins(0, 0, 0, 0);
	box->setAlignment(Qt::AlignBottom);

		QHBoxLayout * top = new QHBoxLayout;
		top->setMargin(0);
		top->setSpacing(0);
		top->setContentsMargins(0, 0, 0, 0);

		QHBoxLayout * bot = new QHBoxLayout;
		bot->setMargin(0);
		bot->setSpacing(0);
		bot->setContentsMargins(0, 0, 0, 0);


		top->addWidget((info_text = new ParupaintInfoBarText(this)));
		top->addWidget((info_tutorial = new ParupaintInfoBarTutorial(this)));
		bot->addWidget((info_status = new ParupaintInfoBarStatus(this)));

		connect(info_text, &QTextBrowser::anchorClicked, this, &ParupaintInfoBar::onStatusClick);
		connect(info_tutorial, &QTextBrowser::anchorClicked, this, &ParupaintInfoBar::onStatusClick);
		connect(info_status, &QTextBrowser::anchorClicked, this, &ParupaintInfoBar::onStatusClick);

	box->addLayout(top, 0);
	box->addLayout(bot, 1);
	this->setLayout(box);

	this->status()->updateTitle();
	this->show();
}
ParupaintInfoBarStatus * ParupaintInfoBar::status()
{
	return info_status;
}
