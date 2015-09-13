
#include "parupaintCanvasBanner.h"
#include <QVBoxLayout>
#include <QDebug>

ParupaintCanvasBanner::ParupaintCanvasBanner(QWidget * parent) : QWidget(parent)
{
	this->setObjectName("CanvasBanner");
	QSize fsize(150, 50);
	this->setFixedSize(fsize);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	layout->setMargin(2);

	label = new QLabel("test", this);
	label->setObjectName("CanvasBannerText");
	label->setAlignment(Qt::AlignCenter);

	layout->addWidget(label, 0, Qt::AlignBottom);
	this->setLayout(layout);

	this->setAttribute(Qt::WA_TransparentForMouseEvents);
	this->hide();
}

void ParupaintCanvasBanner::RefreshLabel() {
	label->setText(pre_text + main_text);
}

void ParupaintCanvasBanner::SetMainText(QString str) {
	main_text = str;
	this->RefreshLabel();
}
void ParupaintCanvasBanner::SetPreText(QString str) {
	pre_text = str;
	this->RefreshLabel();
}

void ParupaintCanvasBanner::Show(qreal time, QString str) {
	show_timer.stop();
	if(!str.isEmpty()) this->SetMainText(str);
	qDebug() << label->width() << label->height();

	if(time == 0) return this->hide();
	else if(time < 0) return this->show();
	else {
		this->show();
		show_timer.start(time);
		connect(&show_timer, &QTimer::timeout, this, &QWidget::hide);

	}
}
