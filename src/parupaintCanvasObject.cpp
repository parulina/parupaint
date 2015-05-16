
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#include "parupaintCanvasObject.h"
#include "panvas/parupaintPanvas.h"
#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"



ParupaintCanvasObject::ParupaintCanvasObject(QGraphicsItem *parent) :QGraphicsObject(parent)
{
	Canvas = new ParupaintPanvas(QSize(256, 256), 1, 1);
}

QRectF ParupaintCanvasObject::boundingRect() const
{
	return QRectF(0, 0, Canvas->GetWidth(), Canvas->GetHeight());
}

void ParupaintCanvasObject::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	QRect exposed = option->exposedRect.adjusted(-1, -1, 1, 1).toAlignedRect();
	
	painter->fillRect(exposed, QColor(255,255,255));

	auto layer = Canvas->GetLayer(0);
	if(layer != nullptr){
		auto frame = layer->GetFrame(0);
		if(frame != nullptr) {
			painter->drawImage(exposed, frame->GetImage());
		}
	}
}

