#ifndef PARUPAINTNEWDIALOG_H
#define PARUPAINTNEWDIALOG_H

#include "parupaintDialog.h"

class QComboBox;

class ParupaintNewDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	QComboBox * width;
	QComboBox * height;


	public:
	ParupaintNewDialog(QWidget * = nullptr);

	signals:
	void NewSignal(int, int, bool=false);
};

#endif
