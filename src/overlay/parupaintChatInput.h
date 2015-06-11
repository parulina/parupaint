#ifndef PARUPAINTCHATINPUT_H
#define PARUPAINTCHATINPUT_H

#include <QLineEdit>

class ParupaintChatInput : public QLineEdit
{
Q_OBJECT
	private:
	void keyPressEvent(QKeyEvent *);

	public:
	ParupaintChatInput(QWidget * = nullptr);

	signals:
	void pageNavigation(bool up);
};

#endif
