#ifndef PARUPAINTSNIPPETS_H
#define PARUPAINTSNIPPETS_H

#include <QImage>
#include <QColor>

class ParupaintSnippets
{
	public:
	static QImage Base64GzipToImage(const QString & base64_image);
	static QString ImageToBase64Gzip(const QImage & img);
};

class ParupaintFillHelper {
	uchar * img_data;
	QImage mask_image;
	QList<QPoint> plist;
	int ww, hh;
	QImage::Format fmt;

	public:
	~ParupaintFillHelper();
	ParupaintFillHelper();
	ParupaintFillHelper(QImage & img);

	QRgb pixel(int x, int y) const{
		return *(QRgb*)(img_data + (4 * (x + (y * ww))));
	}
	QImage image(){
		return QImage(img_data, ww, hh, fmt);
	}
	QImage mask(){
		return mask_image.createAlphaMask();
	}

	QRect fill(int x, int y, const QRgb to) { return fill(x, y, pixel(x, y), to); }
	QRect fill(int x, int y, const QRgb orig, const QRgb to);

};

#endif
