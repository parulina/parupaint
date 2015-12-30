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
		 "<a href=\"http://parupaint.sqnya.se\">official homepage</a>&nbsp;&nbsp;&nbsp;<a href=\"http://github.com/parulina/parupaint\">github</a><br/>"
		 "Welcome to my painting program, parupaint. The program is designed to be quick and light, while still being a nice drawing application.<br/>"
		 "You can read a summary of what this program does on the homepage. Thank you for downloading and using this!<br/>"
		 "<span class=\"alpha-notice\">Please note that this program is currently in alpha and is constantly adding/removing features.</span>"
	 "</p>"
	));
}
QSize ParupaintInfoBarText::minimumSizeHint() const
{
	return QSize(300, 0);
}

ParupaintInfoBarKeys::ParupaintInfoBarKeys(QWidget * parent) : QLabel(parent)
{
	this->setStyleSheet(stylesheet);
	this->setFocusPolicy(Qt::ClickFocus);
	this->setWordWrap(false);
	this->setTextFormat(Qt::RichText);
	this->setTextInteractionFlags(Qt::TextSelectableByMouse);
}
QSize ParupaintInfoBarKeys::minimumSizeHint() const
{
	return QSize(600, 0);
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
			"connected to <a href=\"#connected\">" + (connected_to == "localhost" ? "yourself" : connected_to) + "</a>  "
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
		top->addWidget((info_keys = new ParupaintInfoBarKeys(this)));
		bot->addWidget((info_status = new ParupaintInfoBarStatus(this)));

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

void ParupaintInfoBar::setKeyList(const QStringList & list)
{
	// split up in to columns
	QStringList copy(list);
	int a = 0;
	for(auto i = copy.begin(); i != copy.end(); ++i){
		(*i).append("<br/>");
		if(a && a % 9 == 0){
			i = copy.insert(i, "</td><td>");
		}
		a++;
	}
	copy.prepend("<table id=\"keytable\" cellpadding=5 cellspacing=3><tr valign=\"top\"><td>");
	copy.append("</td></tr></table>");

	info_keys->setText(copy.join("").prepend("<html>").append("</html>"));
}
