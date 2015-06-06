#ifndef PARUPAINTFILEDIALOG_H
#define PARUPAINTFILEDIALOG_H

#include "parupaintDialog.h"

class QLineEdit;
class QLabel;

class ParupaintFileDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	QLineEdit * line_filename;
	QLabel * label_invalid;

	public:
	ParupaintFileDialog(QWidget * = nullptr, QString = "", QString = "File...", QString = "enter filename.");

	signals:
	void EnterSignal(QString);

	private slots:
	void EnterClick();
};


#endif
