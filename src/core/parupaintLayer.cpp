#include "parupaintLayer.h"

#include <QDebug>
#include <QSize>

#include "parupaintPanvas.h" // parentPanvas
#include "parupaintLayerModes.h"

ParupaintLayer::ParupaintLayer(QObject * parent, const QSize & frame_size, int frames) : QObject(parent),
	layer_visible(true), layer_name("layer"), layer_mode(0)
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
bool ParupaintLayer::insertFrame(const QSize & size, ParupaintFrame* at)
{
	return this->insertFrame(new ParupaintFrame(size, this), at);
}
bool ParupaintLayer::insertFrame(const QSize & size, int i)
{
	return this->insertFrame(new ParupaintFrame(size, this), i);
}

bool ParupaintLayer::insertFrame(ParupaintFrame* f, ParupaintFrame* at)
{
	Q_ASSERT_X(f, "insertFrame(at)", "frame to insert is null");
	int i = -1;
	if(at){
		int ii;
		if((ii = frames.indexOf(at)) != -1){
			i = ii;
		}
	}
	return this->insertFrame(f, i);
}
bool ParupaintLayer::insertFrame(ParupaintFrame* f, int i)
{
	Q_ASSERT_X(f, "insertFrame(i)", "frame to insert is null");

	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		frames.insert(i, f);
		f->setParent(this);
		this->connect(f, &QObject::destroyed, this, &ParupaintLayer::removeFrameObject);
		emit onContentChange();
		return true;
	} else {
		qDebug() << "insertFrame(i)" << i << "is out of range";
	}
	return false;
}
bool ParupaintLayer::appendFrame(ParupaintFrame* f)
{
	return this->insertFrame(f, this->frameCount());
}

bool ParupaintLayer::removeFrame(ParupaintFrame* f)
{
	return this->removeFrame(this->frameIndex(f));
}

bool ParupaintLayer::removeFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size()-1)){
		if(!this->isFrameExtended(i)){
			ParupaintFrame * f = frames.takeAt(i);

			f->setParent(nullptr);
			this->disconnect(f, &QObject::destroyed, this, &ParupaintLayer::removeFrameObject);
			delete f;

			emit onContentChange();
			return true;
		}
	} else {
		qDebug() << "removeFrame(i)" << i << "is out of range";
	}
	return false;
}

bool ParupaintLayer::extendFrame(ParupaintFrame* f)
{
	return this->extendFrame(this->frameIndex(f));
}
bool ParupaintLayer::extendFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		ParupaintFrame * frame = this->frameAt(i);
		frames.insert(i, frame);
		emit onContentChange();
		return true;
	} else {
		qDebug() << "extendFrame(i)" << i << "is out of range";
	}
	return false;
}

bool ParupaintLayer::redactFrame(ParupaintFrame* f)
{
	return this->redactFrame(this->frameIndex(f));
}
bool ParupaintLayer::redactFrame(int i)
{
	if(i < 0) i += frames.size();
	if(frames.isEmpty()) i = 0;
	if(i >= 0 && (i <= frames.size())){
		if(!this->isFrameExtended(i)) return false;

		ParupaintFrame * frame = this->frameAt(i);
		switch(this->frameExtendedDirection(frame)){
			case FRAME_EXTENDED_RIGHT: frames.removeAt(i - 1); break;
			default: frames.removeAt(i + 1); break;
		}
		emit onContentChange();
		return true;
	} else {
		qDebug() << "redactFrame(i)" << i << "is out of range";
	}
	return false;
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

		QString str = QString::number(i).rightJustified(3, '0');
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
	if(i < 0 || i >= frames.size()) return nullptr;
	return frames.at(i);
}

void ParupaintLayer::setMode(const QString & textmode)
{
	this->setMode(svgLayerModeToCompositionMode(textmode));
}

void ParupaintLayer::setMode(int mode)
{
	if(mode == layer_mode) return;

	layer_mode = mode;
	emit onModeChange(layer_mode);
}

int ParupaintLayer::mode() const
{
	return layer_mode;
}
QString ParupaintLayer::modeString() const
{
	return compositionModeToString(layer_mode);
}
void ParupaintLayer::setName(const QString & name)
{
	if(name == layer_name) return;

	layer_name = name;
	emit onNameChange(layer_name);
}

QString ParupaintLayer::name() const
{
	return layer_name;
}
void ParupaintLayer::setVisible(bool visible)
{
	if(visible == layer_visible) return;

	layer_visible = visible;
	emit onVisiblityChange(layer_visible);
}

bool ParupaintLayer::visible() const
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
