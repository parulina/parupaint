#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QScrollArea>

class ParupaintCanvasObject;
class ParupaintFlayerList;


enum FlayerStatus {
	FLAYER_STATUS_IDLE,
	FLAYER_STATUS_MOVING
};

class ParupaintFlayer : public QScrollArea
{
	private:
	FlayerStatus FlayerState;
	ParupaintFlayerList * list;
	QPointF OldPosition;

	public:
	ParupaintFlayer(QWidget * = nullptr);
	ParupaintFlayerList * GetList();

	void UpdateFromCanvas(ParupaintCanvasObject *);

	protected:
	void enterEvent(QEvent * );
	void leaveEvent(QEvent * );
	void mousePressEvent(QMouseEvent * );
	void mouseReleaseEvent(QMouseEvent * );
	void mouseMoveEvent(QMouseEvent * );
	void keyPressEvent(QKeyEvent *);
	void keyReleaseEvent(QKeyEvent *);

};

#endif
