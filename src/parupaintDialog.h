#ifndef PARUPAINTDIALOG_H
#define PARUPAINTDIALOG_H

#include <QDialog>

class ParupaintDialog : public QDialog
{
	private:
	QPoint dragpos;
	QString savename;
	void paintEvent(QPaintEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void showEvent(QShowEvent * event);
	void moveEvent(QMoveEvent * event);

	public:
	ParupaintDialog(QWidget * = nullptr, QString = "", QString = "");
	void SetFrameless(bool frameless);
	void SaveGeometry(QString);
	void LoadGeometry(QString);

	void SetSaveName(QString);

};

#endif
