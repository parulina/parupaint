
#include "parupaintDialog.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionTitleBar>

#include <QSettings>
#include <QLabel>
#include <QVBoxLayout>

ParupaintDialog::ParupaintDialog(QWidget * parent, QString title, QString helptext) : QDialog(parent)
{
	QSettings cfg;
	this->setMinimumSize(250, 250);

	bool noframeless = false;
	noframeless = cfg.value("window/noframeless").toBool();

	if(!noframeless) this->setWindowFlags(Qt::FramelessWindowHint);

	this->move(parent->rect().center());
	this->setWindowTitle(title);
	this->setFocusPolicy(Qt::NoFocus);
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	auto * layout = new QVBoxLayout(this);
	layout->setMargin(8);
	
	if(!helptext.isEmpty()){
		auto * label = new QLabel(helptext);
		label->setWordWrap(true);
		label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
		layout->addWidget(label);
		layout->setAlignment(label, Qt::AlignVCenter);
	}
	
	this->setLayout(layout);
}


void ParupaintDialog::mouseMoveEvent(QMouseEvent * event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::mouseMoveEvent(event);

	if (event->buttons() & Qt::LeftButton) {
		this->move(event->globalPos() - dragpos);
		event->accept();
	}
	this->QDialog::mouseReleaseEvent(event);
}

void ParupaintDialog::mousePressEvent(QMouseEvent *event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::mousePressEvent(event);

	if (event->button() == Qt::LeftButton) {
		dragpos = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
	this->QDialog::mousePressEvent(event);
}

void ParupaintDialog::paintEvent(QPaintEvent * event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::paintEvent(event);

	QPainter p(this);
	QStyle* style = this->style();
	QRect active_area = this->rect();
	active_area.adjust(0, 0, -1, -1);
	int titlebar_height = 0;

	QStyleOptionTitleBar t_opt;
	t_opt.initFrom(this);
	titlebar_height = style->pixelMetric(QStyle::PM_TitleBarHeight, &t_opt, this);
	// get correct titlebar height
	t_opt.rect = QRect(0, 0, active_area.width(), titlebar_height);
	/// and rect

	p.setPen(Qt::black);
	QFont font = p.font();
	font.setPointSize(15);
	p.setFont(font);

	p.fillRect(t_opt.rect, this->palette().background());
	p.drawText(t_opt.rect, Qt::AlignCenter, this->windowTitle());
	p.drawRect(t_opt.rect);

	// move contents lower
	active_area.setTopLeft(QPoint(0, titlebar_height));
	this->setContentsMargins(0, titlebar_height, 0, 0);
	p.drawRect(active_area);

	this->QDialog::paintEvent(event);
}



