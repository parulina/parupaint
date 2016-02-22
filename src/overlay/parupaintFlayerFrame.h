#ifndef PARUPAINTFLAYERFRAME_H
#define PARUPAINTFLAYERFRAME_H

#include <QPushButton>

class ParupaintFlayerFrame : public QPushButton
{
Q_OBJECT
	signals:
	void onLayerFrameClick(int l, int f);

	private slots:
	void onClick();
	public:
	int layer, frame;
	ParupaintFlayerFrame(QWidget * = nullptr, int l = 0, int f = 0);
	QSize minimumSizeHint() const;
};

#endif
