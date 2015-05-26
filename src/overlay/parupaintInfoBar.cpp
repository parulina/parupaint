
#include "parupaintInfoBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QFile>

ParupaintInfoBar::ParupaintInfoBar(QWidget * parent) : QWidget(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setObjectName("InfoBar");
	this->setFixedHeight(180);
	this->show();

	QVBoxLayout * box = new QVBoxLayout;
	box->setMargin(0);
	box->setSpacing(0);
	box->setAlignment(Qt::AlignBottom);
	this->setLayout(box);


	QFile file(":resources/chat.css");
	file.open(QFile::ReadOnly);
	QString stylesheet(file.readAll());



	QTextBrowser * keys1 = new QTextBrowser;
	keys1->setObjectName("parupaint-keys1");
	keys1->setFocusPolicy(Qt::ClickFocus);
	keys1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Expanding);
	keys1->document()->setDefaultStyleSheet(stylesheet);
	keys1->setHtml(QStringLiteral(
	"<keys>"
		"<p><key>1-4</key>  <desc>brush size</desc></p>"
		"<p><key>A F</key>  <desc>← frames →</desc></p>"
		"<p><key>S D</key>  <desc>↑ layers ↓</desc></p>"
	"</keys>"

	));


	QTextBrowser * ptext = new QTextBrowser;
	ptext->setObjectName("parupaint-description");
	ptext->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ptext->setOpenLinks(true);
	ptext->setOpenExternalLinks(true);
	ptext->document()->setDefaultStyleSheet(stylesheet);
	ptext->setFocusPolicy(Qt::ClickFocus);
	ptext->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	ptext->setHtml(QStringLiteral("<p class=\"about\">Welcome to my painting program, parupaint. The program is designed to be quick and light, while still able to be a nice drawing platform. It is also able to do animations. You can read a tutorial for this program on my website: <a href=\"http://sqnya.se\">[ sqnya.se ]</a>. Thank you for downloading this and using my creation. :D</p>"));

	QTextBrowser * title = new QTextBrowser;
	title->setObjectName("title");
	title->setMaximumHeight(30);
	title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	title->document()->setDefaultStyleSheet(stylesheet);
	title->setOpenLinks(false);
	title->setFocusPolicy(Qt::ClickFocus);
	title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	title->setHtml(QStringLiteral("<titletext>· PARUPAINT ALPHA 0.6 · [<a href=\"#name\">unnamed</a>]  <a href=\"#dim\">&lt;100x100&gt;</a>  <flayer>{1 : 5}</flayer></titletext>"));

	QLabel * tabtext = new QLabel("hold [tab] for help");
	tabtext->setMaximumHeight(30);
	tabtext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	tabtext->setObjectName("tabhelp");
	tabtext->setFocusPolicy(Qt::NoFocus);




// 	QWidget * topwidget = new QWidget;
// 	topwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
// 	topwidget->setMinimumHeight(0);
// 	topwidget->setAutoFillBackground(true);
// 
// 	QWidget * bottomwidget = new QWidget;
// 	bottomwidget->setMinimumHeight(30);
// 	bottomwidget->setMaximumHeight(30);
// 	bottomwidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
// 	

// 	box->addWidget(topwidget);
// 	box->addWidget(bottomwidget);



	QHBoxLayout * topbox = new QHBoxLayout;
	topbox->setMargin(0);
	topbox->setSpacing(0);
	topbox->setContentsMargins(0, 0, 0, 0);
// 	topwidget->setLayout(topbox);

	QHBoxLayout * bottombox = new QHBoxLayout;
	bottombox->setMargin(0);
	bottombox->setSpacing(0);
	bottombox->setContentsMargins(0, 0, 0, 0);
// 	bottomwidget->setLayout(bottombox);



	topbox->addWidget(ptext);
	topbox->addWidget(keys1);
	bottombox->addWidget(title);
	bottombox->addWidget(tabtext);



	box->addLayout(topbox);
	box->addLayout(bottombox);

}
