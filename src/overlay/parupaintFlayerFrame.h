#ifndef PARUPAINTFLAYERFRAME_H
#define PARUPAINTFLAYERFRAME_H

#include <QPushButton>

class ParupaintFlayerFrame : public QPushButton
{
Q_OBJECT
	public:
	int layer, frame;
	ParupaintFlayerFrame(QWidget * = nullptr);
	QSize minimumSizeHint() const;
};

#endif
