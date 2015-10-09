
#include <QRegularExpression>
#include <QDebug>

//TODO move qcompressor to src/ or src/bundled dir?
#include "../net/qcompressor.h"
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
QImage ParupaintSnippets::toImage(const QString & base64_image)
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
