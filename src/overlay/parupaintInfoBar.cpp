#include "parupaintInfoBar.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextBrowser>
#include <QLabel>
#include <QFile>
#include <QDebug>

#include "../parupaintVersion.h"

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


	QFile file(":resources/chat.css");
	file.open(QFile::ReadOnly);
	QString stylesheet(file.readAll());


	key_list = new QLabel(this);
	key_list->setObjectName("parupaint-keys");
	key_list->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
	key_list->setStyleSheet(stylesheet);
	key_list->setWordWrap(false);
	key_list->setTextFormat(Qt::RichText);
	key_list->setTextInteractionFlags(Qt::TextSelectableByMouse);

	QTextBrowser * ptext = new QTextBrowser;
	ptext->setObjectName("parupaint-description");
	ptext->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	ptext->setOpenLinks(true);
	ptext->setOpenExternalLinks(true);
	ptext->document()->setDefaultStyleSheet(stylesheet);
	ptext->setFocusPolicy(Qt::ClickFocus);
	ptext->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	ptext->setHtml(QStringLiteral(
"<p class=\"about\">"
	"Welcome to my painting program, parupaint. The program is designed to be quick and light, "
	"while still able to be a nice drawing platform. It is also able to do animations."
"<br />"
	"You can read a tutorial for this program on my website: <a href=\"http://sqnya.se#parupaint\">[ sqnya.se ]</a>."
	"Thank you for downloading this and using my creation!"
"<br />"
"<br />"
	"Please note that this program is alpha and is constantly adding/removing features."
"</p>"
	));

	title = new QTextBrowser;
	title->setObjectName("title");
	title->setMaximumHeight(30);
	title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
	title->document()->setDefaultStyleSheet(stylesheet);
	title->setOpenLinks(false);
	title->setFocusPolicy(Qt::ClickFocus);
	title->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
	title->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QLabel * tabtext = new QLabel("hold [tab] for help");
	tabtext->setMaximumHeight(30);
	tabtext->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	tabtext->setObjectName("tabhelp");
	tabtext->setFocusPolicy(Qt::NoFocus);



	QHBoxLayout * topbox = new QHBoxLayout;
	topbox->setMargin(0);
	topbox->setSpacing(0);
	topbox->setContentsMargins(0, 0, 0, 0);

	QHBoxLayout * bottombox = new QHBoxLayout;
	bottombox->setMargin(0);
	bottombox->setSpacing(0);
	bottombox->setContentsMargins(0, 0, 0, 0);


	topbox->addWidget(ptext);
	topbox->addWidget(key_list);
	bottombox->addWidget(title);
	bottombox->addWidget(tabtext);



	box->addLayout(topbox);
	box->addLayout(bottombox);
	this->setLayout(box);

	this->SetCurrentTitle("Home");
	this->SetCurrentDimensions(500, 500);
	this->SetCurrentLayerFrame(1, 5);
	this->ReloadTitle();
}
void ParupaintInfoBar::SetKeyList(QStringList list)
{
	int a = 0;
	for(auto i = list.begin(); i != list.end(); ++i){
		(*i).append("<br/>");
		if(a && a % 9 == 0){ i = list.insert(i, "</td><td>"); }
		a++;
	}
	list.prepend("<table id=\"keytable\" cellpadding=5 cellspacing=3><tr valign=\"top\"><td>");
	list.append("</td></tr></table>");
	QString res = list.join("").prepend("<html>").append("</html>");
	key_list->setText(res);
}

void ParupaintInfoBar::ReloadTitle()
{
	QString text = "<titletext>"
		"· PARUPAINT ALPHA " PARUPAINT_VERSION " · "
		"[<a href=\"#name\">" + current_title + "</a>]  "
		"<a href=\"#dim\">&lt;" + current_dimensions + "&gt;</a>  "
		"<flayer>{" + current_lfstatus + "}</flayer>"
	"</titletext>";
	title->setHtml(text);
}

void ParupaintInfoBar::SetCurrentTitle(QString str)
{
	current_title = str;
	this->ReloadTitle();
}
void ParupaintInfoBar::SetCurrentDimensions(int w, int h)
{
	current_dimensions = QString("%1 x %2").arg(w).arg(h);
	this->ReloadTitle();
}
void ParupaintInfoBar::SetCurrentLayerFrame(int l, int f)
{
	current_lfstatus = QString("%1 : %2").arg(l).arg(f);
	this->ReloadTitle();
}


