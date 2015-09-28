#ifndef PARUPAINTCONNECTIONDIALOG_H
#define PARUPAINTCONNECTIONDIALOG_H

#include "parupaintDialog.h"

class ParupaintLineEdit;

class ParupaintConnectionDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	ParupaintLineEdit * line_nickname;
	ParupaintLineEdit * line_ip;

	public:
	ParupaintConnectionDialog(QWidget * = nullptr);

	signals:
	void ConnectSignal(QString);
	void DisconnectSignal();

	private slots:
	void ConnectClick();
};


#endif
