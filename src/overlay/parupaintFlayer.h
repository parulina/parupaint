#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QScrollArea>

class ParupaintVisualCanvas;
class ParupaintPanvas;
class ParupaintFlayerLayer;


class ParupaintFlayer : public QScrollArea
{
Q_OBJECT
	private:
	QPoint 	old_pos;

	void updateFromCanvas(ParupaintPanvas*);
	void clearHighlight();

	private slots:
	void frame_click();
	public slots:
	void canvas_content_update();
	void current_lf_update();

	signals:
	void onHighlightChange(int l, int f);

	public:
	ParupaintFlayer(QWidget * = nullptr);

	void setHighlightLayerFrame(int l, int f);

	protected:
	void resizeEvent(QResizeEvent*);
	void enterEvent(QEvent * );
	void leaveEvent(QEvent * );
	void mouseMoveEvent(QMouseEvent * );
};

#endif
