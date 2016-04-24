#include "parupaintPanvas.h"

#include <QDebug>
#include <QPainter> // mergedImageFrames
#include <QJsonObject>

#include "parupaintSnippets.h"

#define qtMax(x, y) (x > y ? x : y)

ParupaintPanvasInfo::ParupaintPanvasInfo() :
	framerate(12),
	background_color(Qt::white)
{
}

// loadDefaults to load from QSettings?


ParupaintPanvas::ParupaintPanvas(QObject * parent, const QSize & dim, int layers, int frames) : QObject(parent)
{
	if(!dim.isNull()){
		qDebug() << "Creating panvas with dimensions" << dim;
		this->resize(dim);
	}
	if(layers){
		for(int i = 0; i < layers; i++)
			this->insertLayer(-1, frames);
	}
}

void ParupaintPanvas::removeLayerObject(QObject * object)
{
	ParupaintLayer * layer = qobject_cast<ParupaintLayer*>(object);
	if(layer){
		this->removeLayer(layer);
	}
}

void ParupaintPanvas::updateLayerObject()
{
	ParupaintLayer * layer = qobject_cast<ParupaintLayer*>(sender());
	if(layer){
		emit onCanvasLayerChange(this->layerIndex(layer));
	}
}

void ParupaintPanvas::resize(const QSize & size)
{
	this->info.dimensions = size;
	foreach(auto l, layers){
		l->resize(size);
	}
	emit onCanvasResize(size);
}

void ParupaintPanvas::clearCanvas()
{
	qDeleteAll(layers);
	layers.clear();

	emit onCanvasContentChange();
}
void ParupaintPanvas::newCanvas(int l, int f)
{
	qDebug() << "Creating" << l << "layers with" << f << "frames.";
	this->clearCanvas();
	QList<ParupaintLayer*> list;
	for(int i = 0; i < l; i++){
		list.append(new ParupaintLayer(this, this->dimensions(), f));
	}
	this->newCanvas(list);
}
void ParupaintPanvas::newCanvas(const QList<ParupaintLayer*> & layerlist)
{
	layers = layerlist;
	emit onCanvasContentChange();
}

void ParupaintPanvas::insertLayer(int i, int f)
{
	this->insertLayer(new ParupaintLayer(this, this->dimensions(), f), i);
}

void ParupaintPanvas::insertLayer(ParupaintLayer* l, ParupaintLayer* at)
{
	Q_ASSERT_X(l, "insertLayer(at)", "layer to insert is null");
	int i = -1;
	if(at){
		int ii;
		if((ii = layers.indexOf(at)) != -1){
			i = ii;
		}
	}
	this->insertLayer(l, i);
}
void ParupaintPanvas::insertLayer(ParupaintLayer* l, int i)
{
	Q_ASSERT_X(l, "insertLayer(i)", "layer to insert is null");

	if(i < 0) i += (layers.size());
	if(layers.isEmpty()) i = 0;
	if(i >= 0 && (i <= layers.size())){
		layers.insert(i, l);
		l->setParent(this);
		this->connect(l, &ParupaintLayer::onContentChange, this, &ParupaintPanvas::onCanvasContentChange);
		this->connect(l, &ParupaintLayer::onNameChange, this, &ParupaintPanvas::updateLayerObject);
		this->connect(l, &ParupaintLayer::onModeChange, this, &ParupaintPanvas::updateLayerObject);
		this->connect(l, &ParupaintLayer::onVisiblityChange, this, &ParupaintPanvas::updateLayerObject);

		this->connect(l, &QObject::destroyed, this, &ParupaintPanvas::removeLayerObject);
		emit onCanvasContentChange();
	} else {
		qDebug() << "insertLayer(i)" << i << "is out of range";
	}
}
void ParupaintPanvas::appendLayer(ParupaintLayer* l)
{
	this->insertLayer(l, this->layerCount());
}

void ParupaintPanvas::removeLayer(ParupaintLayer* l)
{
	this->removeLayer(this->layerIndex(l));
}

void ParupaintPanvas::removeLayer(int i)
{
	if(i < 0) i += layers.size();
	if(layers.isEmpty()) i = 0;
	if(i >= 0 && i <= layers.size()){
		ParupaintLayer * l = layers.takeAt(i);
		l->setParent(nullptr);
		this->disconnect(l, &ParupaintLayer::onContentChange, this, &ParupaintPanvas::onCanvasContentChange);
		this->disconnect(l, &ParupaintLayer::onNameChange, this, &ParupaintPanvas::updateLayerObject);
		this->disconnect(l, &ParupaintLayer::onModeChange, this, &ParupaintPanvas::updateLayerObject);
		this->disconnect(l, &ParupaintLayer::onVisiblityChange, this, &ParupaintPanvas::updateLayerObject);

		this->disconnect(l, &QObject::destroyed, this, &ParupaintPanvas::removeLayerObject);
		// i'm not sure if this is okay...
		delete l;
		emit onCanvasContentChange();
	} else {
		qDebug() << "removeLayer(i)" << i << "is out of range";
	}
}
int ParupaintPanvas::layerIndex(ParupaintLayer* l)
{
	Q_ASSERT_X(l, "removeLayer(l)", "layer to remove is null");
	int i = layers.indexOf(l);
	if(i == -1) qDebug() << "layerIndex" << l << "is not found";
	return i;
}
ParupaintLayer * ParupaintPanvas::layerAt(int i) const
{
	if(layers.isEmpty()) return nullptr;
	return layers.at(i);
}

int ParupaintPanvas::totalFrameCount()
{
	int count = 0;
	foreach(auto l, layers){
		count = qtMax(l->frameCount(), count);
	}
	return count;
}
int ParupaintPanvas::layerCount() const
{
	return layers.length();
}

QList<QImage> ParupaintPanvas::mergedImageFrames(bool rendered)
{
	QList<QImage> images;
	for(auto f = 0; f < this->totalFrameCount(); f++){
		QImage img(this->dimensions(), QImage::Format_ARGB32);
		img.fill(this->info.background_color.rgba());
		images.append(img);
	}

	for(auto l = 0; l < this->layerCount(); l++){
		ParupaintLayer * layer = this->layerAt(l);
		if(!layer) continue;
		if(!layer->visible()) continue;

		QList<QImage> layerFrames = layer->imageFrames(rendered);
		for(int i = 0; i < layerFrames.length(); i++){
			QPainter painter(&images[i]);
			painter.setCompositionMode(static_cast<QPainter::CompositionMode>(layer->mode()));

			painter.drawImage(images[i].rect(), layerFrames[i], layerFrames[i].rect());
		}
	}
	return images;
}
QImage ParupaintPanvas::mergedImage(bool rendered)
{
	qDebug() << "mergedImage... all frames will be merged.";
	QImage image(this->dimensions(), QImage::Format_ARGB32);
	image.fill(this->backgroundColor().rgba());

	QPainter painter(&image);
	// why the fuck did i write this code???
	// why would i want to merge ALL of the frames? 0_o
	foreach(const QImage &img, this->mergedImageFrames(rendered)){
		painter.drawImage(image.rect(), img, img.rect());
	}

	return image;
}

void ParupaintPanvas::setProjectName(const QString & name)
{
	if(this->info.name == name) return;

	this->info.name = name;
	emit onCanvasChange();
}
void ParupaintPanvas::setFrameRate(qreal framerate)
{
	if(this->info.framerate == framerate) return;

	this->info.framerate = framerate;
	emit onCanvasChange();
}

void ParupaintPanvas::setBackgroundColor(const QColor color)
{
	if(this->info.background_color == color) return;

	this->info.background_color = color;
	emit onCanvasChange();
	emit onCanvasBackgroundChange();
}

const QString ParupaintPanvas::projectDisplayName() const
{
	return this->projectName().isEmpty() ? "Unnamed" : this->projectName();
}
const QString & ParupaintPanvas::projectName() const
{
	return this->info.name;
}
qreal ParupaintPanvas::frameRate() const
{
	return this->info.framerate;
}
QColor ParupaintPanvas::backgroundColor() const
{
	return this->info.background_color;
}

const QSize & ParupaintPanvas::dimensions() const
{
	return this->info.dimensions;
}

QJsonObject ParupaintPanvas::json() const
{
	QJsonObject layers;
	for(int l = 0; l < this->layerCount(); l++) {
		ParupaintLayer * layer = this->layerAt(l);

		QJsonObject frames;
		for(int f = 0; f < layer->frameCount(); f++) {
			if(!layer->isFrameReal(f)) continue;

			ParupaintFrame * frame = layer->frameAt(f);
			QString label = layer->frameLabel(f);

			frames[label] = QJsonObject{
				{"opacity", frame->opacity()}
			};
		}
		// rightJustified so they are in order
		layers.insert(QString::number(l).rightJustified(3, '0'), QJsonObject{
			{"visible", layer->visible()},
			{"name", layer->name()},
			{"mode", layer->mode()},
			{"frames", frames}
		});
	}
	return QJsonObject{
		{"canvasWidth", this->dimensions().width()},
		{"canvasHeight", this->dimensions().height()},
		{"backgroundColor", this->backgroundColor().name(QColor::HexArgb)},
		{"frameRate", this->frameRate()},
		{"projectName", this->projectName()},
		{"layers", layers}
	};
}

void ParupaintPanvas::loadJson(const QJsonObject & obj)
{
	this->clearCanvas();
	this->resize(QSize(obj.value("canvasWidth").toInt(180), obj.value("canvasHeight").toInt(180)));
	this->setFrameRate(obj.value("frameRate").toDouble(12));
	this->setProjectName(obj.value("projectName").toString());
	this->setBackgroundColor(QColor(obj.value("backgroundColor").toString()));

	QJsonObject layers = obj.value("layers").toObject();
	foreach(const QString & lk, layers.keys()) {

		const QJsonObject & lo = layers.value(lk).toObject();
		// lk = layer key
		// lo = layer object

		ParupaintLayer * layer = new ParupaintLayer;
		layer->setVisible(lo.value("visible").toBool(true));
		layer->setName(lo.value("name").toString());
		layer->setMode(lo.value("mode").toInt());

		const QJsonObject & ff = lo.value("frames").toObject();

		foreach(const QString & fk, ff.keys()){
			const QJsonObject & fo = ff.value(fk).toObject();

			int extended = 0;
			if(fk.contains('-')){
				extended = fk.section('-', 1, 1).toInt();
			}

			// create a new frame with the image
			ParupaintFrame * frame = new ParupaintFrame(this->dimensions());
			frame->setOpacity(fo.value("opacity").toDouble(1.0));

			layer->insertFrame(frame, layer->frameCount());

			// extend the frame as far as it wants
			if(extended > 512) extended = 512;
			for(; extended > 0; extended--){
				layer->extendFrame(frame);
			}
		}
		this->insertLayer(layer, this->layerCount());
	}
}
