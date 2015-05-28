#ifndef PARUPAINTCHATINPUT_H
#define PARUPAINTCHATINPUT_H

#include <QLineEdit>

class ParupaintChatInput : public QLineEdit
{
	private:
	void keyPressEvent(QKeyEvent *);

	public:
	ParupaintChatInput(QWidget * = nullptr);
};

#endif
