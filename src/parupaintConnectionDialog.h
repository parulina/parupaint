#ifndef PARUPAINTCONNECTIONDIALOG_H
#define PARUPAINTCONNECTIONDIALOG_H

#include "parupaintDialog.h"

class QLineEdit;

class ParupaintConnectionDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	QLineEdit * line_nickname;	
	QLineEdit * line_ip;	

	public:
	ParupaintConnectionDialog(QWidget * = nullptr);

	signals:
	void ConnectSignal(QString);
	private slots:
	void ConnectClick();
};


#endif
