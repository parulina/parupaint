#ifndef PARUPAINTFILEDIALOG_H
#define PARUPAINTFILEDIALOG_H

#include <QFileDialog>

class QLineEdit;
class QLabel;

enum ParupaintFileDialogType {
	dialogTypeOpen = 0,
	dialogTypeSaveAs
};

class ParupaintFileDialog : public QFileDialog
{
Q_OBJECT
	public:
	ParupaintFileDialog(ParupaintFileDialogType type, QWidget * = nullptr);
};


#endif
