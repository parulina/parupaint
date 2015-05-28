
#include "parupaintChatInput.h"

#include <QKeyEvent>

ParupaintChatInput::ParupaintChatInput(QWidget * parent) : QLineEdit(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setObjectName("ChatEntry");
}

void ParupaintChatInput::keyPressEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Return){
		if(!this->text().isEmpty()){
			emit returnPressed();
		}
		event->accept();
		return;
	}
	QLineEdit::keyPressEvent(event);
}
