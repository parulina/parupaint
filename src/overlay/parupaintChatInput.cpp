
#include "parupaintChatInput.h"

#include <QKeyEvent>

ParupaintChatInput::ParupaintChatInput(QWidget * parent) : QLineEdit(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setObjectName("ChatEntry");
}

void ParupaintChatInput::focusInEvent(QFocusEvent* e)
{
	emit focusIn();
	this->QLineEdit::focusInEvent(e);
}
void ParupaintChatInput::focusOutEvent(QFocusEvent* e)
{
	emit focusOut();
	this->QLineEdit::focusOutEvent(e);
}


void ParupaintChatInput::keyPressEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Return){
		emit returnPressed();
		event->accept();
		return;
	}
	emit keyPress(event);
	QLineEdit::keyPressEvent(event);
}
