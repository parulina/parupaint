
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "parupaintCanvasObject.h"
#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"


ParupaintCanvasObject::ParupaintCanvasObject()
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

	auto layer = GetLayer(0);
	if(layer != nullptr){
		auto frame = layer->GetFrame(0);
		if(frame != nullptr) {
			painter->drawImage(exposed, frame->GetImage());
		}
	}
}

