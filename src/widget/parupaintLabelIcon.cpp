#include "parupaintLabelIcon.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QDebug>

ParupaintLabelIcon::ParupaintLabelIcon(const QPixmap & icon, const QString & label, QWidget * parent) :
	QFrame(parent),
	label_icon(new QLabel(this)),
	label_text(new QLabel(this))
{
	this->setIcon(icon);
	this->setText(label);

	QHBoxLayout * layout = new QHBoxLayout;
		layout->addWidget(label_icon, 0, Qt::AlignLeft | Qt::AlignVCenter);
		layout->addWidget(label_text, 1, Qt::AlignLeft | Qt::AlignVCenter);
	this->setLayout(layout);
}

void ParupaintLabelIcon::setIcon(const QPixmap & icon)
{
	label_icon->setPixmap(icon);
}
void ParupaintLabelIcon::setText(const QString & text)
{
	label_text->setText(text);
}
