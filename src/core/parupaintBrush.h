#ifndef PARUPAINTBRUSH_H
#define PARUPAINTBRUSH_H

#include <QObject>
#include <QPointF>
#include <QPen>
#include <QColor>

enum ParupaintBrushPattern {
	BrushPatternNone = 0,

	BrushShadingPattern,   // Dense5Pattern
	BrushHighlightPattern, // Dense6Pattern
	BrushCrossPattern,     // DiagCrossPattern
	BrushGridPattern,      // Own pattern

	BrushPatternMax
};
enum ParupaintBrushTool {
	BrushToolNone = 0,
	BrushToolFloodFill,
	BrushToolOpacityDrawing,
	BrushToolLine,
	BrushToolMax // don't change this!
};

class ParupaintBrush : public QObject
{
Q_OBJECT
	private:
	QColor 	brush_color;
	QString brush_name;
	QPointF brush_position;

	qreal 	brush_size;
	qreal 	brush_pressure;

	int 	brush_layer;
	int 	brush_frame;

	bool 	brush_drawing;
	int 	brush_tool;
	int 	brush_pattern;

	signals:
	void onNameChange(const QString &);
	void onColorChange(const QColor &);
	void onPositionChange(const QPointF &);
	void onToolChange(int);
	void onPatternChange(int);
	void onLayerChange(int);
	void onFrameChange(int);

	public:
	ParupaintBrush(QObject * = nullptr, qreal size = 1.0, const QColor = QColor(-1, -1, -1, -1), const QString & name = "", int tool = ParupaintBrushTool::BrushToolNone, int pattern = ParupaintBrushPattern::BrushPatternNone);

	virtual void setName(const QString &);
	virtual void setColor(const QColor &);
	virtual void setSize(qreal);
	virtual void setPosition(const QPointF &);
	void setX(qreal);
	void setY(qreal);
	void setPressure(qreal);
	void setLayerFrame(int l, int f);
	void setLayer(int l);
	void setFrame(int f);
	void setDrawing(bool);
	void setTool(int);
	void setPattern(int);

	const QString & name() const;
	const QColor & color() const;
	qreal size() const;
	const QPointF & position() const;
	qreal pressure() const;
	int layer() const;
	int frame() const;
	bool drawing() const;
	int tool() const;
	int pattern() const;

	// handy funcs
	qreal x();
	qreal y();
	QRgb rgba();
	QPen pen();
	QPoint pixelPosition();
	qreal pressureSize();
	QString colorString();
	QRectF localRect();
	QImage patternImage();

	void copyTo(ParupaintBrush &);
};

#endif
