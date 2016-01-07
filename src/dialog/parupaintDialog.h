#ifndef PARUPAINTDIALOG_H
#define PARUPAINTDIALOG_H

#include <QDialog>

class ParupaintDialog : public QDialog
{
	private:
	QPoint drag_pos;
	QString savename;

	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void showEvent(QShowEvent * event);
	void moveEvent(QMoveEvent * event);

	void saveGeometry();

	protected:
	void loadGeometry(const QString & name);

	public:
	ParupaintDialog(QWidget * = nullptr, QString = "dialog");
	void setFrameless(bool frameless);

	void SetSaveName(QString);

};

#endif
