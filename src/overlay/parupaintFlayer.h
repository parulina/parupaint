#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QScrollArea>

class ParupaintVisualCanvas;
class ParupaintPanvas;
class ParupaintFlayerLayer;


class ParupaintFlayerList : public QFrame
{
Q_OBJECT
	public:
	ParupaintFlayerList(QWidget * = nullptr);
};

class ParupaintFlayer : public QScrollArea
{
Q_OBJECT
	private:
	QPoint 	old_pos;
	ParupaintFlayerList * layers;

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
	void mouseMoveEvent(QMouseEvent * );
	bool event(QEvent * event);
	QSize minimumSizeHint() const;
};

#endif
