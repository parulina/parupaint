#ifndef PARUPAINTFLAYER_H
#define PARUPAINTFLAYER_H

#include <QListWidget>

class ParupaintVisualCanvas;
class ParupaintPanvas;
class ParupaintFlayerLayer;


class ParupaintFlayer : public QListWidget
{
Q_OBJECT
	private:
	QPoint 	old_pos;

	void updateFromCanvas(ParupaintPanvas*);
	void clearHighlight();
	ParupaintFlayerLayer * flayer(int i);

	public slots:
	void reloadCanvasSlot();
	void updateCanvasSlot();

	signals:
	void onHighlightChange(int l, int f);
	void onLayerVisibleChange(int l, bool visible);
	void onLayerNameChange(int l, const QString & name);
	void onLayerModeChange(int l, int mode);

	public:
	ParupaintFlayer(QWidget * = nullptr);
	void setHighlightLayerFrame(int l, int f);

	// from outside
	void setLayerVisible(int l, bool visible);
	void setLayerName(int l, const QString & name);

	protected:
	void mouseMoveEvent(QMouseEvent * );
	void mouseReleaseEvent(QMouseEvent * );
	QSize minimumSizeHint() const;
};

#endif
