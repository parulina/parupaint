#ifndef PARUPAINTNETJOINPROMPT_H
#define PARUPAINTNETJOINPROMPT_H

#include <QFrame>

class ParupaintNetJoinPrompt : public QFrame
{
Q_OBJECT
	public:
	ParupaintNetJoinPrompt(QWidget * = nullptr);

	signals:
	void wantJoin();
};
#endif
