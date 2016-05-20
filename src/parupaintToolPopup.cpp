#include "parupaintToolPopup.h"

#include <QPainter>

ParupaintToolPopup::ParupaintToolPopup(QWidget * parent) :
	ParupaintPopupSelector(parent)
{
	QImage tools = QImage(":/resources/icons.png").convertToFormat(QImage::Format_ARGB32);
	for(int i = 0; i < tools.width()/32; i++){
		QImage sub = tools.copy(i*32, 0, 32, 32).scaledToWidth(64);
		if(i == 0) sub.fill(0);
		this->addPixmap(QPixmap::fromImage(sub));
	}
}
