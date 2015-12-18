#ifndef PARUPAINTSCROLLBAR_H
#define PARUPAINTSCROLLBAR_H

#include <QScrollBar>

class ParupaintScrollBar : public QScrollBar
{
Q_OBJECT
	QPoint old_pos;
	bool use_direction_signal;

	void mousePressEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);

	signals:
	void directionMove(const QPoint &);

	public:
	ParupaintScrollBar(Qt::Orientation orientation, QWidget * = nullptr, bool direction = false);
	void setUseDirection(bool direction);
};

#endif
