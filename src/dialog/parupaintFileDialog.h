#ifndef PARUPAINTFILEDIALOG_H
#define PARUPAINTFILEDIALOG_H

#include <QFileDialog>
#include <QLabel>

class QLineEdit;
class QLabel;

enum ParupaintFileDialogType {
	dialogTypeOpen = 0,
	dialogTypeSaveAs
};

class ParupaintFileDialogPreview : public QLabel
{
Q_OBJECT
	public:
	ParupaintFileDialogPreview(QWidget * = nullptr);
	void setFilePreview(const QString & file);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

class ParupaintFileDialog : public QFileDialog
{
Q_OBJECT
	public:
	ParupaintFileDialog(ParupaintFileDialogType type, QWidget * = nullptr);
};


#endif
