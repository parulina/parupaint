#ifndef PARUPAINTPOPUPSELECTOR_H
#define PARUPAINTPOPUPSELECTOR_H

#include <QWidget>

class ParupaintPopupSelector : public QWidget
{
Q_OBJECT
	signals:
	void selectIndex(int index);
	void done();

	private:
	int focusedIndex();
	protected:
	void leaveEvent(QEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void keyPressEvent(QKeyEvent * event);

	public:
	ParupaintPopupSelector(QWidget * = nullptr);

	void addWidget(QWidget * widget);
	void addPixmap(const QPixmap & pixmap);
	void focusIndex(int index);
};

#endif
