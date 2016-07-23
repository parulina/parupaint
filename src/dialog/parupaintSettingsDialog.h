#ifndef PARUPAINTSETTINGSDIALOG_H
#define PARUPAINTSETTINGSDIALOG_H

#include "parupaintDialog.h"

class ParupaintSettingsDialog : public ParupaintDialog
{
Q_OBJECT
	private slots:
	void confirmConfigClear();
	public:
	ParupaintSettingsDialog(QWidget * = nullptr);

	signals:
	void pixelgridChanged(bool);
	void nameChanged(const QString &);
	void sessionPasswordChanged(const QString &);
	void cursorModeChanged(bool);
	void viewportFastUpdateChanged(bool);
	void keyBindOpen();
	void configCleared();
};

#endif
