
#include "parupaintLineEdit.h"

ParupaintLineEdit::ParupaintLineEdit(QWidget * parent, const QString placeholder) : QLineEdit(parent)
{
	this->setAttribute(Qt::WA_MacShowFocusRect, 0);
	if(!placeholder.isEmpty()) this->setPlaceholderText(placeholder);
}
