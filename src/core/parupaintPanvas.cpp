#include "parupaintPanvas.h"

#include <QDebug>
#include <QPainter> // mergedImageFrames

#define qtMax(x, y) (x > y ? x : y)

ParupaintPanvasInfo::ParupaintPanvasInfo()
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
	for(int i = 0; i < f; i++){
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
		this->connect(l, &QObject::destroyed, this, &ParupaintPanvas::removeLayerObject);
		emit onCanvasContentChange();
	} else {
		qDebug() << "insertLayer(i)" << i << "is out of range";
	}
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
ParupaintLayer * ParupaintPanvas::layerAt(int i)
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
int ParupaintPanvas::layerCount()
{
	return layers.length();
}

QList<QImage> ParupaintPanvas::mergedImageFrames(bool rendered)
{
	QList<QImage> images;
	for(auto f = 0; f < this->totalFrameCount(); f++){
		QImage img(this->dimensions(), QImage::Format_ARGB32);
		img.fill(0);
		images.append(img);
	}

	for(auto l = 0; l < this->layerCount(); l++){
		auto * layer = this->layerAt(l);
		if(!layer) continue;

		QList<QImage> layerFrames = layer->imageFrames(rendered);
		for(int i = 0; i < layerFrames.length(); i++){
			QPainter painter(&images[i]);
			painter.drawImage(images[i].rect(), layerFrames[i], layerFrames[i].rect());
		}
	}
	return images;
}

void ParupaintPanvas::setProjectName(const QString & name)
{
	this->info.name = name;
	emit onCanvasChange();
}
void ParupaintPanvas::setFrameRate(qreal framerate)
{
	this->info.framerate = framerate;
	emit onCanvasChange();
}

const QString & ParupaintPanvas::projectName()
{
	return this->info.name;
}
qreal ParupaintPanvas::frameRate()
{
	return this->info.framerate;
}

const QSize & ParupaintPanvas::dimensions() const
{
	return this->info.dimensions;
}
