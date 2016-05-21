#include "parupaintInfoBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QFile> // stylesheet
#include <QDebug>
#include <QTabWidget>

#include "../widget/parupaintScrollBar.h"

QString stylesheet = "";

ParupaintInfoBarText::ParupaintInfoBarText(QWidget * parent) : QTextBrowser(parent)
{
	this->document()->setDocumentMargin(2);
	this->document()->setDefaultStyleSheet(stylesheet);
	this->setFocusPolicy(Qt::ClickFocus);
	this->setOpenLinks(true);
	this->setOpenExternalLinks(true);
	this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));
}

ParupaintInfoBarTutorial::ParupaintInfoBarTutorial(QWidget * parent) : QTextBrowser(parent)
{
	this->document()->setDocumentMargin(2);
	this->document()->setDefaultStyleSheet(stylesheet);
	this->setFocusPolicy(Qt::ClickFocus);
	this->setOpenLinks(true);
	this->setOpenExternalLinks(true);
	this->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);
	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));
	this->setHtml(QStringLiteral(
	));
}

ParupaintInfoBarTabWidget::ParupaintInfoBarTabWidget(QWidget * parent) :
	QTabWidget(parent)
{
	// for some reason setting background color through QSS
	// doesn't even work.....
	this->setFocusPolicy(Qt::NoFocus);
}

QSize ParupaintInfoBarTabWidget::minimumSizeHint() const
{
	return QSize(300, 0);
}

ParupaintInfoBarStatus::ParupaintInfoBarStatus(QWidget * parent) : QFrame(parent)
{
	this->setFixedHeight(30);
	this->setFocusPolicy(Qt::ClickFocus); // NoFocus

	connected_text = new QLabel("");
	QLabel * prefix = new QLabel("#");

	connected_text->setObjectName("ConnectedTo");

	QHBoxLayout * layout = new QHBoxLayout;
		layout->addStretch(1);
		layout->addWidget(prefix, 0, Qt::AlignCenter);
		layout->addWidget(connected_text, 0, Qt::AlignCenter);
		layout->addStretch(1);
	this->setLayout(layout);
}

void ParupaintInfoBarStatus::setConnectedText(const QString & text)
{
	connected_text->setText(text);
}

QSize ParupaintInfoBarStatus::minimumSizeHint() const
{
	return QSize(300, 30);
}

// InfoBar
ParupaintInfoBar::ParupaintInfoBar(QWidget * parent) : QFrame(parent)
{
	this->setFocusPolicy(Qt::NoFocus);

	QFile file(":/resources/chat.css");
	file.open(QFile::ReadOnly);
	stylesheet = file.readAll();

	QVBoxLayout * box = new QVBoxLayout;
		box->setMargin(0);
		box->setSpacing(0);
		box->setContentsMargins(0, 0, 0, 0);

		info_status = new ParupaintInfoBarStatus(this);
		info_text = new ParupaintInfoBarText;
		info_tutorial = new ParupaintInfoBarText;

		info_text->setHtml(QStringLiteral(
		 "<p class=\"about\">"
			"<h3>PARUPAINT</h3>"
			"<a href=\"http://parupaint.sqnya.se\">official homepage</a>&nbsp;&nbsp;&nbsp;<a href=\"http://github.com/parulina/parupaint\">github</a><br/>"
			"Welcome to my painting program, parupaint. The program is designed to be quick and light, while still being a nice drawing application. You can read a summary of what this program does on the homepage. Thank you for downloading and using this!<br/>"
			"<span class=\"notice\">Please note that parupaint is in alpha and features are constantly changed.</span>"
		 "</p>"
		));
		info_tutorial->setHtml(QStringLiteral(
		 "<p class=\"mini-tutorial\">"
			 "<h3>QUICK START (DEFAULT KEYS)</h3>"
			 "<a href=\"http://parupaint.sqnya.se/tutorial.html\">full tutorial</a><br/>"
			 "Press Ctrl+K to quicksave, and Ctrl+M for settings.<br/>"
			 "<span class=\"notice\">The settings has a full key reference.</span><br/>"
			 "Hold Space to move canvas, and Ctrl+Space to zoom. Hold Shift+Space to change brush size. Press W for halftones, Q for fill tool, T to test fill.<br/>"
			 "<u>Press Shift+Tab to hide this!</u>"
		 "</p>"
		));


		ParupaintInfoBarTabWidget * tab_widget = new ParupaintInfoBarTabWidget(this);
			tab_widget->addTab(info_text, "parupaint");
			tab_widget->addTab(info_tutorial, "quick start");

		box->addWidget(tab_widget, 1, Qt::AlignTop);
		box->addWidget(info_status, 0, Qt::AlignBottom);

	this->setLayout(box);
}
void ParupaintInfoBar::setConnectedText(const QString & text)
{
	info_status->setConnectedText(text);
}

QSize ParupaintInfoBar::minimumSizeHint() const
{
	return QSize(200, 20);
}

QSize ParupaintInfoBar::sizeHint() const
{
	return QSize(65535, 200);
}
