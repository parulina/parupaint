#ifndef PARUPAINTCHATINPUT_H
#define PARUPAINTCHATINPUT_H

#include <QLineEdit>

class ParupaintChatInput : public QLineEdit
{
Q_OBJECT
	private:
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent*);
	void focusOutEvent(QFocusEvent*);

	public:
	ParupaintChatInput(QWidget * = nullptr);

	signals:
	void pageNavigation(bool up, bool faster=false);
	void focusIn();
	void focusOut();
};

#endif
