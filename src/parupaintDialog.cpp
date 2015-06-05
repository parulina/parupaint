
#include "parupaintDialog.h"
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionTitleBar>

#include <QLabel>
#include <QVBoxLayout>

ParupaintDialog::ParupaintDialog(QWidget * parent, QString title, QString helptext) : QDialog(parent)
{
	this->setMinimumSize(250, 250);

	this->setWindowFlags(Qt::FramelessWindowHint);
	this->setWindowModality(Qt::ApplicationModal);
	this->move(parent->rect().center());
	this->setWindowTitle(title);
	this->setFocusPolicy(Qt::NoFocus);
	this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

	auto * layout = new QVBoxLayout(this);
	layout->setMargin(8);
	
		auto * label = new QLabel(helptext);
		label->setWordWrap(true);

	layout->addWidget(label);
	layout->setAlignment(label, Qt::AlignTop);
	
	this->setLayout(layout);
}


void ParupaintDialog::mouseMoveEvent(QMouseEvent * event)
{
	if (event->buttons() & Qt::LeftButton) {
		this->move(event->globalPos() - dragpos);
		event->accept();
	}
	this->QDialog::mouseReleaseEvent(event);
}

void ParupaintDialog::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton) {
		dragpos = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
	this->QDialog::mousePressEvent(event);
}

void ParupaintDialog::paintEvent(QPaintEvent * event)
{
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



