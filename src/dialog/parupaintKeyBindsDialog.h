#ifndef PARUPAINTKEYBINDSDIALOG_H
#define PARUPAINTKEYBINDSDIALOG_H

#include <QPushButton>
#include "parupaintDialog.h"

class ParupaintKeyBindButton : public QPushButton
{
Q_OBJECT
	public:
	ParupaintKeyBindButton(const QString & label, QWidget * = nullptr);
	QSize minimumSizeHint() const;
};

class ParupaintKeys;
class ParupaintKeyBindsDialog : public ParupaintDialog
{
Q_OBJECT
	private:
	ParupaintKeys * keys_pointer;
	ParupaintKeyBindButton * focused_keybutton;

	private slots:
	void focusKeyButton();

	void keyPressEvent(QKeyEvent * event);
	public:
	ParupaintKeyBindsDialog(ParupaintKeys * keys, QWidget * = nullptr);
};

#endif
