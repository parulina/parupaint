#ifndef PARUPAINTBRUSHGLASS_H
#define PARUPAINTBRUSHGLASS_H

#include <QObject>
#include <QList>
#include <QColor>

class ParupaintBrush;

class ParupaintBrushGlass : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintBrush*> brushes;

	// holds the original brush when
	// selecting a brush >= 2
	int swapped_brush;
	int current_brush;

	signals:
	void onBrushChange();
	void onBrushColorChange(QColor);
	void onCurrentBrushChange(int currentBrush);

	public slots:
	void color_change(QColor);

	public:
	ParupaintBrushGlass(QObject * = nullptr);
	void addBrush(ParupaintBrush *);

	ParupaintBrush * brush();
	ParupaintBrush * toggleBrush(int newbrush);
	ParupaintBrush * setBrush(int newbrush);
	int brushNum();
};

#endif
