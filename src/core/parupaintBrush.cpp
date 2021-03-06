#include "parupaintBrush.h"
#include "parupaintPatterns.h"

// brush!

#include <QDebug>
#include <QPainter>
#include <QPen>
#include <QtMath>

ParupaintBrush::ParupaintBrush(QObject * parent,
		qreal size, const QColor color, const QString & name, int tool, int pattern) :
	QObject(parent),
	brush_color(color), brush_name(name), brush_position(0, 0),
	brush_size(size), brush_pressure(0.0),
	brush_layer(0), brush_frame(0),
	brush_drawing(false),
	brush_tool(tool), brush_pattern(pattern)
{
}

void ParupaintBrush::setName(const QString & name) {
	const QString old_name = this->brush_name;
	this->brush_name = name;
	if(name != old_name) emit onNameChange(name);
}
void ParupaintBrush::setColor(const QColor & color) {
	const QColor old_color = this->brush_color;
	this->brush_color = color;
	if(color != old_color) emit onColorChange(color);
}
void ParupaintBrush::setSize(qreal size) {
	if(size <= 1) size = 1;
	if(size >= 512) size = 512;
	this->brush_size = size;
}
void ParupaintBrush::setPosition(const QPointF & pos) {
	const QPointF old_position = this->brush_position;
	this->brush_position = pos;
	if(pos != old_position) emit onPositionChange(pos);
}
void ParupaintBrush::setX(qreal x){ this->brush_position.setX(x); }
void ParupaintBrush::setY(qreal y){ this->brush_position.setY(y); }

void ParupaintBrush::setPressure(qreal pressure) {
	if(pressure < 0) pressure = 0; 
	if(pressure > 1) pressure = 1;
	this->brush_pressure = pressure;
}
void ParupaintBrush::setLayerFrame(int l, int f) {
	this->setLayer(l);
	this->setFrame(f);
}
void ParupaintBrush::setLayer(int l) {
	int old_layer = this->brush_layer;

	this->brush_layer = l;
	if(l != old_layer) emit onLayerChange(l);
}
void ParupaintBrush::setFrame(int f) {
	int old_frame = this->brush_frame;

	this->brush_frame = f;
	if(f != old_frame) emit onFrameChange(f);
}
void ParupaintBrush::setDrawing(bool drawing) {
	this->brush_drawing = drawing;
}
void ParupaintBrush::setTool(int tool) {

	int old_tool = this->brush_tool;
	if(tool < ParupaintBrushTool::BrushToolNone)
		tool = ParupaintBrushTool::BrushToolNone;
	if(tool >= ParupaintBrushTool::BrushToolMax)
		tool = ParupaintBrushTool::BrushToolMax;

	this->brush_tool = tool;
	if(tool != old_tool) emit onToolChange(tool);
}
void ParupaintBrush::setPattern(int pattern) {

	int old_pattern = this->brush_pattern;
	if(pattern < ParupaintBrushPattern::BrushPatternNone)
		pattern = ParupaintBrushPattern::BrushPatternNone;
	if(pattern >= ParupaintBrushPattern::BrushPatternMax)
		pattern = ParupaintBrushPattern::BrushPatternMax;

	this->brush_pattern = pattern;
	if(pattern != old_pattern) emit onPatternChange(pattern);
}

const QString & ParupaintBrush::name() const { return this->brush_name; }
const QColor & ParupaintBrush::color() const { return this->brush_color; }
qreal ParupaintBrush::size() const { return this->brush_size; }
const QPointF & ParupaintBrush::position() const { return this->brush_position; }
qreal ParupaintBrush::pressure() const { return this->brush_pressure; }
int ParupaintBrush::layer() const { return this->brush_layer; }
int ParupaintBrush::frame() const { return this->brush_frame; }
bool ParupaintBrush::drawing() const { return this->brush_drawing; }
int ParupaintBrush::tool() const { return this->brush_tool; }
int ParupaintBrush::pattern() const { return this->brush_pattern; }

qreal ParupaintBrush::x() { return this->brush_position.x(); }
qreal ParupaintBrush::y() { return this->brush_position.y(); }
QRgb ParupaintBrush::rgba() { return this->brush_color.rgba(); }
QPen ParupaintBrush::pen()
{
	QPen pen(this->color());
	pen.setCapStyle(Qt::RoundCap);
	pen.setWidthF(this->pressureSize());
	return pen;
}

QPoint ParupaintBrush::pixelPosition() {
	return QPoint(qFloor(brush_position.x()), qFloor(brush_position.y()));
}
qreal ParupaintBrush::pressureSize() {
	return brush_size * brush_pressure;
}
QString ParupaintBrush::colorString() {
	return this->brush_color.name(QColor::HexArgb);
}

QRectF ParupaintBrush::localRect()
{
	return QRectF(-pressureSize(), -pressureSize(), pressureSize(), pressureSize());
}

QImage ParupaintBrush::patternImage()
{
	if(this->pattern() == ParupaintBrushPattern::BrushPatternNone) return QImage();
	return parupaintPattern(this->pattern()-1, this->color());
}

void ParupaintBrush::copyTo(ParupaintBrush & brush)
{
	brush.setName(this->name());
	brush.setColor(this->color());
	brush.setPosition(this->position());
	brush.setSize(this->size());
	brush.setPressure(this->pressure());
	brush.setLayerFrame(this->layer(), this->frame());
	brush.setDrawing(this->drawing());
	brush.setTool(this->tool());
	brush.setPattern(this->pattern());
}
