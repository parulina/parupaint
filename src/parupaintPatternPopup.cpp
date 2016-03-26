#include "parupaintPatternPopup.h"

#include <QPainter>

#include "core/parupaintPatterns.h"

ParupaintPatternPopup::ParupaintPatternPopup(QWidget * parent) :
	ParupaintPopupSelector(parent)
{
	QImage pattern, temp_img(64, 64, QImage::Format_ARGB32);
	int p = -1;
	do{
		temp_img.fill(0);

		if(!pattern.isNull()) {
			QPainter painter(&temp_img);
			painter.fillRect(temp_img.rect(), QBrush(pattern));
			painter.end();
		}

		this->addPixmap(QPixmap::fromImage(temp_img));
		p++;

	} while(!(pattern = parupaintPattern(p)).isNull());
}
