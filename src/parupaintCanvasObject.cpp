
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QDebug>

#include "parupaintCanvasObject.h"
#include "core/parupaintLayer.h"
#include "core/parupaintFrame.h"

ParupaintCanvasObject::~ParupaintCanvasObject()
{
	
}

ParupaintCanvasObject::ParupaintCanvasObject() :
	CurrentLayer(0), CurrentFrame(0), Preview(false),
	checker(":/resources/checker.png")
{
	this->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}

void ParupaintCanvasObject::New(QSize s, _lint l, _fint f)
{
	QSize old = ParupaintPanvas::GetSize();
	this->ParupaintPanvas::New(s, l, f);
	emit ResizeSignal(old, s);
	NewCache();
	RedrawCache();
}
void ParupaintCanvasObject::Resize(QSize s)
{
	QSize old = ParupaintPanvas::GetSize();
	this->ParupaintPanvas::Resize(s);
	emit ResizeSignal(old, s);
	NewCache();
	RedrawCache();
}

QRectF ParupaintCanvasObject::boundingRect() const
{
	return QRectF(0, 0, GetWidth(), GetHeight());
}

void ParupaintCanvasObject::NewCache()
{
	cache = QPixmap(GetWidth(), GetHeight());
}

void ParupaintCanvasObject::RedrawCache()
{
	this->RedrawCache(this->boundingRect().toRect());
}

void ParupaintCanvasObject::RedrawCache(QRect area)
{
	if(cache.isNull()) NewCache();
	QPainter painter(&cache);
	auto layer = GetLayer(CurrentLayer);

	const QPointF ppp(area.x() % checker.width(), area.y() % checker.height());
	painter.drawTiledPixmap(area, checker, ppp);

	if(!IsPreview()){
		// "draw debug mode"
		painter.setOpacity(0.6);
	}
	for(auto i = 0; i < GetNumLayers(); i++){
		// draw previous frames
		auto layer2 = GetLayer(i);
		if(layer2 && CurrentFrame < layer2->GetNumFrames()){
			auto frame2 = layer2->GetFrame(CurrentFrame);
			if(frame2){
				painter.drawImage(area, frame2->GetImage(), area);
			}
		}
	}

	if(!IsPreview()){
		painter.fillRect(area, Qt::white);
		painter.setOpacity(0.8);
		if(layer != nullptr){
			auto frame = layer->GetFrame(CurrentFrame);
			if(frame != nullptr) {
				painter.drawImage(area, frame->GetImage(), area);
			}
		}
	}
}

void ParupaintCanvasObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	const QRect exposed = option->exposedRect.toAlignedRect();
	painter->drawPixmap(exposed, cache, exposed);
}

bool ParupaintCanvasObject::IsPreview() const
{
	return Preview;
}

void ParupaintCanvasObject::SetPreview(bool b)
{
	Preview = b;
	RedrawCache();
}
void ParupaintCanvasObject::TogglePreview()
{
	SetPreview(!IsPreview());
}

void ParupaintCanvasObject::SetLayerFrame(_lint layer, _fint frame)
{
	if(GetNumLayers() == 0) return;

	if(layer >= GetNumLayers()) layer = GetNumLayers()-1;
	CurrentLayer = layer;

	if(GetLayer(layer)->GetNumFrames() == 0) return;
	if(frame >= GetLayer(layer)->GetNumFrames()) frame = GetLayer(layer)->GetNumFrames()-1;
	CurrentFrame = frame;

	RedrawCache();
	emit CurrentSignal(int(CurrentLayer), int(CurrentFrame));
}

void ParupaintCanvasObject::FixLayerFrame()
{
	this->SetLayerFrame(CurrentLayer, CurrentFrame);
}

void ParupaintCanvasObject::AddLayerFrame(int layer, int frame)
{
	if(int(CurrentLayer) + layer < 0) layer = 0;
	if(int(CurrentFrame) + frame < 0) frame = 0;

	if(int(CurrentLayer) + layer >= GetNumLayers()) layer = -(GetNumLayers()-1);
	if(int(CurrentFrame) + frame >= GetLayer(CurrentLayer)->GetNumFrames()) frame = -(GetLayer(CurrentLayer)->GetNumFrames()-1);
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

const QPixmap & ParupaintCanvasObject::GetCache() const
{
	return cache;
}
