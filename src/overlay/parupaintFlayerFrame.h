#ifndef PARUPAINTFLAYERFRAME_H
#define PARUPAINTFLAYERFRAME_H

#include <QPushButton>

class ParupaintFlayerFrame : public QPushButton
{
	private:

	public:
	int	Index;
	bool 	Extended;
	ParupaintFlayerFrame(QWidget * = nullptr);
};

#endif
