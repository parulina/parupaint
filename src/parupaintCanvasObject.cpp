
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "parupaintCanvasObject.h"
#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"


ParupaintCanvasObject::ParupaintCanvasObject() :
	CurrentLayer(0), CurrentFrame(0)
{
	
}

void ParupaintCanvasObject::Resize(QSize s)
{
	QSize old = ParupaintPanvas::GetSize();
	this->ParupaintPanvas::Resize(s);
	emit ResizeSignal(old, s);
}

QRectF ParupaintCanvasObject::boundingRect() const
{
	return QRectF(0, 0, GetWidth(), GetHeight());
}

void ParupaintCanvasObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	QRect exposed = option->exposedRect.adjusted(-1, -1, 1, 1).toAlignedRect();
	painter->fillRect(exposed, QColor(255,255,255));

	auto layer = GetLayer(CurrentLayer);
	if(layer != nullptr){
		auto frame = layer->GetFrame(CurrentFrame);
		if(frame != nullptr) {
			painter->drawImage(exposed, frame->GetImage());
		}
	}
}

void ParupaintCanvasObject::SetLayerFrame(_lint layer, _fint frame)
{
	if(layer >= GetNumLayers()) layer = GetNumLayers()-1;
	CurrentLayer = layer;

	if(frame >= GetLayer(layer)->GetNumFrames()) frame = GetLayer(layer)->GetNumFrames()-1;
	CurrentFrame = frame;

	emit CurrentSignal(int(CurrentLayer), int(CurrentFrame));
}

void ParupaintCanvasObject::AddLayerFrame(int layer, int frame)
{
	if(int(CurrentLayer) + layer < 0) layer = 0;
	if(int(CurrentFrame) + frame < 0) frame = 0;

	if(int(CurrentLayer) + layer >= GetNumLayers()) layer = 0;
	if(int(CurrentFrame) + frame >= GetLayer(CurrentLayer)->GetNumFrames()) frame = 0;
	SetLayerFrame(CurrentLayer+layer, CurrentFrame+frame);
}

_lint ParupaintCanvasObject::GetCurrentLayer()
{
	return CurrentLayer;
}

_fint ParupaintCanvasObject::GetCurrentFrame()
{
	return CurrentFrame;
}
