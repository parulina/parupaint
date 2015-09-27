
#include <QDebug>

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
