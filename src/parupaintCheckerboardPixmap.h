#ifndef PARUPAINTCHECKERBOARDPIXMAP_H
#define PARUPAINTCHECKERBOARDPIXMAP_H

#include <QPixmap>
#include <QImage>

class ParupaintCheckerboardPixmap : public QPixmap
{
	public:	ParupaintCheckerboardPixmap(QColor c1, QColor c2) : QPixmap(32, 32)
		{
			QImage img(this->size(), QImage::Format_ARGB32);

			QRgb col1 = c1.rgb(), col2 = c2.rgb();
			for(int y = 0; y < 32; y ++){
				for(int x = 0; x < 32; x ++){
					if(y <= 16)
						img.setPixel(x, y, (x <= 16) ? col1 : col2);
					else
						img.setPixel(x, y, (x <= 16) ? col2 : col1);
				}
			}
			this->fill(Qt::transparent);
			this->convertFromImage(img);
		}
};

#endif
