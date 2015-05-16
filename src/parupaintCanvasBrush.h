#ifndef PARUPAINTCANVASBRUSH_H
#define PARUPAINTCANVASBRUSH_H

#include <QColor>

// todo: extends ParupaintBrush?
class ParupaintCanvasBrush {
	private:
	QColor 	color;
	float 	size;
	QString	name;

	public:
	ParupaintCanvasBrush();

	void SetName(QString & str);
	void SetSize(float s);
	void SetColor(QColor &  col);
	QPen ToPen();

	float Size();
};

#endif
