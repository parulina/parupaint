
#include <QRegularExpression>
#include <QBuffer>
#include <QDebug>

//TODO move qcompressor to src/ or src/bundled dir?
#include "../bundled/qcompressor.h"
#include "parupaintSnippets.h"

QColor ParupaintSnippets::toColor(const QString & hex)
{
	QString h = hex;
	h = h.remove('#');

	const QString alpha = h.right((h.length() == 4 || h.length() == 8) ? (h.length() / 4) : 0);
	QColor color(hex.left(hex.length() - alpha.length()));
	if(!alpha.isEmpty()){
		color.setAlpha(alpha.toInt(nullptr, 16));
	}
	return color;
}
QString ParupaintSnippets::toHex(const QColor & col)
{
	return ("#"+ 
		("0" + QString::number(col.red(), 16)).right(2) +
		("0" + QString::number(col.green(), 16)).right(2) +
		("0" + QString::number(col.blue() , 16)).right(2) +
		("0" + QString::number(col.alpha(), 16)).right(2)).toUpper();
}
QImage ParupaintSnippets::Base64GzipToImage(const QString & base64_image)
{
	QRegularExpression exp("^data:image/(png|jpg|jpeg|bmp);base64,");
	QRegularExpressionMatch match = exp.match(base64_image);
	if(!match.hasMatch()) return QImage();
	QString mime = match.captured(1);
	QString image = base64_image.section(',', 1, 1);

	QByteArray data = QByteArray::fromBase64(image.toUtf8());
	QByteArray uncompressed;
	QCompressor::gzipDecompress(data, uncompressed);
	QImage img;
	img.loadFromData(uncompressed, mime.toStdString().c_str());
	return img;
}
QString ParupaintSnippets::ImageToBase64Gzip(const QImage & img)
{
	QByteArray pngData;
	QBuffer buf(&pngData);
	buf.open(QIODevice::WriteOnly);
	img.save(&buf, "png");
	buf.close();

	QByteArray compressed;
	QCompressor::gzipCompress(pngData, compressed);

	return "data:image/png;base64," + QString(compressed.toBase64());
}

ParupaintFillHelper::~ParupaintFillHelper() {
	if(img_data != nullptr) delete [] img_data;
}
ParupaintFillHelper::ParupaintFillHelper() : img_data(nullptr), ww(0), hh(0) {
}
ParupaintFillHelper::ParupaintFillHelper(QImage & img) : ParupaintFillHelper() {
	img_data = new uchar[img.byteCount()];
	ww = img.width(), hh = img.height();
	fmt = img.format();
	memcpy(img_data, img.bits(), img.byteCount());

	mask_image = QImage(ww, hh, fmt);
	mask_image.fill(0);

	plist.clear();
	plist.reserve(ww * hh);
}


QRect ParupaintFillHelper::fill(int x, int y, const QRgb orig, const QRgb to){
	QRect rect = QRect(0, 0, ww, hh);
	if(fmt != QImage::Format_ARGB32) return QRect();
	if(orig == to) return QRect();

	if(!rect.contains(x, y)) return QRect();
	if(pixel(x, y) != orig) return QRect();

	plist = {QPoint(x, y)};
	QRect filled_rect = QRect(x, y, 1, 1);

	bool *above_pixels = new bool[ww];
	bool *below_pixels = new bool[ww];

	// A B G R
	while(!plist.isEmpty()){
		const QPoint p = plist.takeFirst();
		const int ty = p.y();
		int tx = p.x(), tx2 = p.x();

		if(ty > filled_rect.bottom()) filled_rect.setBottom(ty);
		if(ty < filled_rect.top()) filled_rect.setTop(ty);

		while(tx > 0){
			tx--;
			if(pixel(tx, ty) != orig){
				tx++;
				break;
			}
		}
		while(tx2 < ww){
			tx2++;
			if(tx2 >= ww) break; // .pixel(.width) doesn't work for some reason
			if(pixel(tx2, ty) != orig){
				break;
			}
		}

		above_pixels[(tx > 0 ? tx - 1 : 0)] = false;
		below_pixels[(tx > 0 ? tx - 1 : 0)] = false;

		if(tx < filled_rect.left()) filled_rect.setLeft(tx);
		if(tx2 > filled_rect.right()) filled_rect.setRight(tx2);

		for(int x = tx; x < tx2; x++){
			above_pixels[x] = below_pixels[x] = false;
			*(QRgb*)(img_data + (4 * (x + (ty * ww)))) = to;
			*(QRgb*)(mask_image.bits() + (4 * (x + (ty * ww)))) = to;

			// first check if the above pixel is valid to change
			// if it is, then check if the neighboring pixel to the above pixel is planned to change.
			// i mean, if it is, there's no point adding it to the queue.
			if(ty > 0 && pixel(x, ty-1) == orig){
				above_pixels[x] = true;
				if(!(x > 0 && above_pixels[x-1] == true)) { 
					plist.append(QPoint(x, ty-1));
				}
			}
			if(ty < hh-1 && pixel(x, ty+1) == orig) {
				below_pixels[x] = true;
				if(!(x > 0 && below_pixels[x-1] == true)) { 
					plist.append(QPoint(x, ty+1));
				}
			}
		}
	}
	delete [] above_pixels;
	delete [] below_pixels;

	return filled_rect;
}
