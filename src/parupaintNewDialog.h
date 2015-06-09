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
	int cwidth;
	int cheight;

	public:
	ParupaintNewDialog(QWidget * = nullptr);
	void setOriginalDimensions(int, int);

	signals:
	void NewSignal(int, int, bool);
};

#endif
