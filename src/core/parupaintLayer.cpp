#include "parupaintLayer.h"

#include <QDebug>
#include <QSize>

#include "parupaintPanvas.h" // parentPanvas

ParupaintLayer::ParupaintLayer(QObject * parent, const QSize & frame_size, int frames) : QObject(parent),
	layer_visible(true)
{
	for(int i = 0; i < frames; i++){
		this->insertFrame(frame_size, -1);
	}
}

ParupaintPanvas * ParupaintLayer::parentPanvas()
{
	return qobject_cast<ParupaintPanvas*>(this->parent());
}

void ParupaintLayer::removeFrameObject(QObject * object)
{
	ParupaintFrame * frame = qobject_cast<ParupaintFrame*>(object);
	if(frame){
		this->removeFrame(frame);
	}
}

void ParupaintLayer::resize(const QSize & size)
{
	foreach(auto f, frames){
		f->resize(size);
	}
}
void ParupaintLayer::insertFrame(const QSize & size, ParupaintFrame* at)
{
	this->insertFrame(new ParupaintFrame(size, this), at);
}
void ParupaintLayer::insertFrame(const QSize & size, int i)
{
	this->insertFrame(new ParupaintFrame(size, this), i);
}

void ParupaintLayer::insertFrame(ParupaintFrame* f, ParupaintFrame* at)
{
	Q_ASSERT_X(f, "insertFrame(at)", "frame to insert is null");
	int i = -1;
	if(at){
		int ii;
		if((ii = frames.indexOf(at)) != -1){
			i = ii;
		}
	}
	this->insertFrame(f, i);
}
void ParupaintLayer::insertFrame(ParupaintFrame* f, int i)
{
	Q_ASSERT_X(f, "insertFrame(i)", "frame to insert is null");

	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		frames.insert(i, f);
		f->setParent(this);
		this->connect(f, &QObject::destroyed, this, &ParupaintLayer::removeFrameObject);
		emit onContentChange();
	} else {
		qDebug() << "insertFrame(i)" << i << "is out of range";
	}
}
void ParupaintLayer::appendFrame(ParupaintFrame* f)
{
	this->insertFrame(f, this->frameCount());
}

void ParupaintLayer::removeFrame(ParupaintFrame* f)
{
	this->removeFrame(this->frameIndex(f));
}

void ParupaintLayer::removeFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		ParupaintFrame * f = frames.takeAt(i);
		if(!this->isFrameExtended(f)){
			f->setParent(nullptr);
			this->disconnect(f, &QObject::destroyed, this, &ParupaintLayer::removeFrameObject);
			delete f;
		}
		emit onContentChange();
	} else {
		qDebug() << "removeFrame(i)" << i << "is out of range";
	}
}

void ParupaintLayer::extendFrame(ParupaintFrame* f)
{
	this->extendFrame(this->frameIndex(f));
}
void ParupaintLayer::extendFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		ParupaintFrame * frame = this->frameAt(i);
		frames.insert(i, frame);
		emit onContentChange();
	} else {
		qDebug() << "extendFrame(i)" << i << "is out of range";
	}
}

void ParupaintLayer::redactFrame(ParupaintFrame* f)
{
	this->redactFrame(this->frameIndex(f));
}
void ParupaintLayer::redactFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		if(!this->isFrameExtended(i)) return;

		ParupaintFrame * frame = this->frameAt(i);
		switch(this->frameExtendedDirection(frame)){
			case FRAME_EXTENDED_RIGHT: frames.removeAt(i - 1); break;
			default: frames.removeAt(i + 1); break;
		}
		emit onContentChange();
	} else {
		qDebug() << "redactFrame(i)" << i << "is out of range";
	}
}

bool ParupaintLayer::isFrameExtended(ParupaintFrame* f)
{
	return this->isFrameExtended(this->frameIndex(f));
}
bool ParupaintLayer::isFrameExtended(int i)
{
	return (this->frameExtendedDirection(i) != FRAME_NOT_EXTENDED);
}

bool ParupaintLayer::isFrameReal(ParupaintFrame* f)
{
	return this->isFrameReal(this->frameIndex(f));
}
bool ParupaintLayer::isFrameReal(int i)
{
	int direction = this->frameExtendedDirection(i);
	return (direction == FRAME_NOT_EXTENDED || direction == FRAME_EXTENDED_LEFT);
}

int ParupaintLayer::frameExtendedDirection(ParupaintFrame* f)
{
	return this->frameExtendedDirection(this->frameIndex(f));
}
int ParupaintLayer::frameExtendedDirection(int i)
{
	int d = 0;
	if(i >= 0 && i < frames.length()) {
		if(i > 0 && frames.at(i-1) == frames.at(i)) {
			d++;
		}
		if(i < frameCount()-1 && frames.at(i+1) == frames.at(i)) {
			d += 2;
		}
	}
	return d;
}

QChar ParupaintLayer::frameExtendedChar(ParupaintFrame* f)
{
	return this->frameExtendedChar(this->frameIndex(f));
}
QChar ParupaintLayer::frameExtendedChar(int i)
{
	switch(this->frameExtendedDirection(i)){
		case FRAME_NOT_EXTENDED:
			return QChar('x');
		case FRAME_EXTENDED_LEFT:
			return QChar('<');
		case FRAME_EXTENDED_MIDDLE:
			return QChar('=');
		case FRAME_EXTENDED_RIGHT:
			return QChar('>');
		default:
			return QChar(' ');
	}
}

QString ParupaintLayer::frameLabel(ParupaintFrame* f)
{
	return this->frameLabel(this->frameIndex(f));
}
QString ParupaintLayer::frameLabel(int i)
{
	if(i >= 0 && i < frames.length()) {

		QString str = QString::number(i);
		if(this->frameExtendedDirection(i) == FRAME_EXTENDED_LEFT){
			int end = i + 1;
			while(this->frameExtendedDirection(end) != FRAME_EXTENDED_RIGHT){
				end++;
			}
			str.append(QString("-%2").arg(end-i));
		}
		return str;
	}
	return QString("null");
}

int ParupaintLayer::frameIndex(ParupaintFrame* f)
{
	Q_ASSERT_X(f, "removeLayer(f)", "layer to remove is null");

	int i = frames.indexOf(f);
	if(i == -1) qDebug() << "frameIndex" << f << "was not found";
	return i;
}
ParupaintFrame * ParupaintLayer::frameAt(int i)
{
	if(frames.isEmpty()) return nullptr;
	return frames.at(i);
}

void ParupaintLayer::setVisible(bool visible)
{
	layer_visible = visible;
	emit onVisiblityChange(layer_visible);
}

bool ParupaintLayer::visible()
{
	return layer_visible;
}

int ParupaintLayer::frameCount()
{
	return frames.length();
}
int ParupaintLayer::realFrameCount()
{
	int count = 0;
	foreach(auto f, frames){
		if(this->isFrameReal(f)) count++;
	}
	return count;
}

QList<QImage> ParupaintLayer::imageFrames(bool rendered)
{
	QList<QImage> images;
	foreach(auto f, frames){
		images.append(rendered ? f->renderedImage() : f->image());
	}
	return images;
}
