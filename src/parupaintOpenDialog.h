#ifndef PARUPAINTOPENDIALOG_H
#define PARUPAINTOPENDIALOG_H

#include "parupaintDialog.h"

class QLineEdit;
class QLabel;

class ParupaintOpenDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	QLineEdit * line_filename;
	QLabel * label_invalid;

	void showEvent(QShowEvent * event);
	public:
	ParupaintOpenDialog(QWidget * = nullptr);

	signals:
	void EnterSignal(QString);

	private slots:
	void EnterClick();
};


#endif
