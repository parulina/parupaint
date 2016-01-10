#ifndef PARUPAINTPASSWORDDIALOG_H
#define PARUPAINTPASSWORDDIALOG_H

#include "parupaintDialog.h"

class ParupaintLineEdit;
class ParupaintPasswordDialog : public ParupaintDialog
{
Q_OBJECT
	ParupaintLineEdit * password_line;
	public:
	ParupaintPasswordDialog(QWidget * = nullptr);

	private slots:
	void checkPassword();
	signals:
	void enterPassword(const QString & password);
	void knockKnock();
};

#endif
