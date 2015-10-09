#ifndef PARUPAINTSNIPPETS_H
#define PARUPAINTSNIPPETS_H

#include <QImage>
#include <QColor>

class ParupaintSnippets
{
	public:
	static QColor toColor(const QString & hex);
	static QImage toImage(const QString & base64_image);
};

#endif
