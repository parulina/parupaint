#include "parupaintNetJoinPrompt.h"

#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

ParupaintNetJoinPrompt::ParupaintNetJoinPrompt(QWidget * parent) :
	QFrame(parent)
{
	this->setFocusPolicy(Qt::NoFocus);

	QLabel * join_label = new QLabel("you are currently spectating.", this);
	QPushButton * join_button = new QPushButton("join in", this);
	connect(join_button, &QPushButton::pressed, this, &ParupaintNetJoinPrompt::wantJoin);

	join_button->setFocusPolicy(Qt::NoFocus);
	join_button->setCursor(Qt::PointingHandCursor);

	QHBoxLayout *layout = new QHBoxLayout;
	layout->setMargin(0);
	layout->setSizeConstraint(QLayout::SetFixedSize);
		layout->addWidget(join_label);
		layout->addWidget(join_button);
	this->setLayout(layout);
}
