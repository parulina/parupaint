#ifndef PARUPAINTSETTINGSDIALOG_H
#define PARUPAINTSETTINGSDIALOG_H

#include "parupaintDialog.h"

class ParupaintSettingsDialog : public ParupaintDialog
{
Q_OBJECT
	public:
	ParupaintSettingsDialog(QWidget * = nullptr);
	signals:
	void pixelgridChanged(bool);
};

#endif
