#ifndef PARUPAINTCHATINPUT_H
#define PARUPAINTCHATINPUT_H

#include "../widget/parupaintLineEdit.h"

class ParupaintChatInput : public ParupaintLineEdit
{
Q_OBJECT
	private:
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

	public:
	ParupaintChatInput(QWidget * = nullptr);

	signals:
	void keyPress(QKeyEvent *);
	void focusIn();
	void focusOut();
};

#endif
