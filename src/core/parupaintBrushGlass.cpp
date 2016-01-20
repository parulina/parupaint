#include "parupaintBrushGlass.h"

// ParupaintBrushGlass holds brushes in a glass...
// like in real life

#include <QDebug>
#include <QSettings>

#include "parupaintSnippets.h"
#include "parupaintBrush.h"

ParupaintBrushGlass::ParupaintBrushGlass(QObject * parent) : QObject(parent),
	swapped_brush(-1), current_brush(0)
{
	this->addBrush(new ParupaintBrush(this,  1, Qt::black, "pencil"));
	this->addBrush(new ParupaintBrush(this, 30, Qt::transparent, "eraser"));
	this->addBrush(new ParupaintBrush(this,  4, Qt::black, "brush"));
	this->addBrush(new ParupaintBrush(this, 30, Qt::white, "cotape"));
	this->addBrush(new ParupaintBrush(this, 10, Qt::red, "red"));
	this->addBrush(new ParupaintBrush(this, 10, Qt::green, "green"));
	this->addBrush(new ParupaintBrush(this, 10, Qt::blue, "blue"));
}

void ParupaintBrushGlass::color_change(QColor color)
{
	this->brush()->setColor(color);
}

void ParupaintBrushGlass::addBrush(ParupaintBrush * brush)
{
	if(!brush) return;
	brushes.append(brush);

	brush->setParent(this);
	connect(brush, &ParupaintBrush::onColorChange, this, &ParupaintBrushGlass::onBrushColorChange);
}

ParupaintBrush * ParupaintBrushGlass::brush()
{
	return (current_brush >= 0 && current_brush < brushes.size()) ?
		brushes[current_brush] : brushes.first();
}
ParupaintBrush * ParupaintBrushGlass::toggleBrush(int newbrush)
{
	if(!(newbrush >= 0 && newbrush < brushes.size())) return nullptr;
	if(!(current_brush >= 0 && current_brush < brushes.size())) return nullptr;

	if(swapped_brush == -1){
		swapped_brush = current_brush;
	} else {
		if(current_brush == newbrush)
			newbrush = swapped_brush;
		swapped_brush = -1;
	}
	return this->setBrush(newbrush);
}
ParupaintBrush * ParupaintBrushGlass::setBrush(int newbrush)
{
	if(!(newbrush >= 0 && newbrush < brushes.size())) return nullptr;
	if(!(current_brush >= 0 && current_brush < brushes.size())) return nullptr;

	brushes[newbrush]->setDrawing(false);
	brushes[newbrush]->setPosition(this->brush()->position());
	brushes[newbrush]->setLayerFrame(this->brush()->layer(), this->brush()->frame());

	current_brush = newbrush;
	emit onCurrentBrushChange(current_brush);
	return this->brush();
}

int ParupaintBrushGlass::brushNum()
{
	return current_brush;
}
bool ParupaintBrushGlass::isToggling()
{
	return (swapped_brush != -1);
}
void ParupaintBrushGlass::clearToggle()
{
	swapped_brush = -1;
}

void ParupaintBrushGlass::saveBrushes()
{
	QSettings cfg;
	cfg.beginGroup("brushes");
	cfg.remove("");

	int i = 0;
	foreach(ParupaintBrush * b, brushes){
		QString key = QString("%1_%2").
			arg(i, 3, 10, QChar('0')).arg(b->name());

		cfg.setValue(key, QStringList{
			b->colorString(),
			QString::number(b->size()),
			QString::number(b->tool())
		});
		i++;
	}
}

void ParupaintBrushGlass::loadBrushes()
{
	QSettings cfg;
	cfg.beginGroup("brushes");

	if(cfg.childKeys().isEmpty()) return;

	qDeleteAll(brushes);
	brushes.clear();

	foreach(const QString & key, cfg.childKeys()){
		if(!key.contains('_')) continue;

		int num = key.section('_', 0, 0).toInt();
		QString name = key.section('_', 1);

		QStringList val = cfg.value(key).toStringList();
		if(val.size() != 3) continue;

		QColor col = ParupaintSnippets::toColor(val[0]);
		qreal size = val[1].toDouble();
		int tool = val[2].toInt();

		// color, size, tool
		this->addBrush(new ParupaintBrush(this, size, col, name, tool));

	}
}

