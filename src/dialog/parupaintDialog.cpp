#include "parupaintDialog.h"

#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionTitleBar>
#include <QMargins>
#include <QSettings>
#include <QLabel>
#include <QVBoxLayout>

ParupaintDialog::ParupaintDialog(QWidget * parent, QString title) : QDialog(parent)
{
	QSettings cfg;
	this->setMinimumSize(250, 250);

	Qt::WindowFlags flags = Qt::Dialog;
	if(cfg.value("client/frameless").toBool()) flags = Qt::Widget | Qt::FramelessWindowHint;
	this->setWindowFlags(flags);

	this->setWindowTitle(title);
	this->setAttribute(Qt::WA_DeleteOnClose);
	this->show();
}

void ParupaintDialog::saveGeometry()
{
	if(savename.isEmpty()) return;

	QSettings cfg;
	cfg.setValue("window/" + savename, this->QDialog::saveGeometry());
	
}
void ParupaintDialog::loadGeometry(const QString & name)
{
	savename = name;

	QSettings cfg;
	restoreGeometry(cfg.value("window/" + savename).toByteArray());
}

void ParupaintDialog::setFrameless(bool frameless)
{
	bool cframe = this->windowFlags().testFlag(Qt::FramelessWindowHint);
	QPoint new_pos = this->pos();

	if(this->parentWidget()){
		if(!cframe && frameless) {
			new_pos = this->pos() - this->parentWidget()->pos();

		} else if(cframe && !frameless) {
			new_pos = this->parentWidget()->pos() + this->pos();
		}
	}

	if(frameless) this->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);
	if(!frameless) this->setWindowFlags(Qt::Dialog);

	this->show();
	this->move(new_pos);
}


void ParupaintDialog::mouseMoveEvent(QMouseEvent * event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::mouseMoveEvent(event);

	if (event->buttons() & Qt::LeftButton) {
		this->move(event->globalPos() - drag_pos);
		event->accept();
	}
	this->QDialog::mouseReleaseEvent(event);
}

void ParupaintDialog::mousePressEvent(QMouseEvent *event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::mousePressEvent(event);

	if (event->button() == Qt::LeftButton) {
		drag_pos = event->globalPos() - frameGeometry().topLeft();
		event->accept();
	}
	this->QDialog::mousePressEvent(event);
}
void ParupaintDialog::mouseReleaseEvent(QMouseEvent *event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::mouseReleaseEvent(event);

	this->saveGeometry();
	this->QDialog::mousePressEvent(event);
}

void ParupaintDialog::paintEvent(QPaintEvent * event)
{
	this->setContentsMargins(0, 0, 0, 0);
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::paintEvent(event);

	QPainter p(this);
	QStyle* style = this->style();
	QRect active_area = this->rect().adjusted(0, 0, -1, -1);
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

void ParupaintDialog::showEvent(QShowEvent * event)
{
	if(!this->windowFlags().testFlag(Qt::FramelessWindowHint)) return this->QDialog::showEvent(event);

	if(this->parentWidget()){
		QRect parent_rect = this->parentWidget()->rect();
		QRect this_rect = this->geometry();

		if(!parent_rect.contains(this_rect, true)){

			int w_change = 0;
			int h_change = 0;

			if(this_rect.right() > parent_rect.right())
				w_change += this_rect.right() - parent_rect.right();

			if(this_rect.bottom() > parent_rect.bottom())
				w_change += this_rect.bottom() - parent_rect.bottom();

			if(this_rect.left() < parent_rect.left())
				w_change += this_rect.left() - parent_rect.left();

			if(this_rect.top() < parent_rect.top())
				w_change += this_rect.top() - parent_rect.top();

			this->move(this_rect.x() - w_change, this_rect.y() - h_change);
		}
	}
	this->QDialog::showEvent(event);
}

void ParupaintDialog::moveEvent(QMoveEvent * event)
{
	this->QDialog::moveEvent(event);
}
