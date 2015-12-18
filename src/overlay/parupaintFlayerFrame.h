#ifndef PARUPAINTFLAYERFRAME_H
#define PARUPAINTFLAYERFRAME_H

class ParupaintFrame;

#include <QPushButton>

class ParupaintFlayerFrame : public QPushButton
{
Q_OBJECT
	public:
	int layer, frame;
	ParupaintFlayerFrame(ParupaintFrame * frame = nullptr, QWidget * = nullptr);
	QSize minimumSizeHint() const;
};

#endif
