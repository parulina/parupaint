#ifndef PARUPAINTDIALOG_H
#define PARUPAINTDIALOG_H

#include <QDialog>

class ParupaintDialog : public QDialog
{
	private:
	QPoint dragpos;
	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);

	public:
	ParupaintDialog(QWidget * = nullptr);

};

#endif
