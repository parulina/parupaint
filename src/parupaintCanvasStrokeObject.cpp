
#include "stroke/parupaintStrokeStep.h"
#include "parupaintCanvasStrokeObject.h"

#include "parupaintCanvasObject.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QLineF>

// this is always at 0,0 and has the bounding rect of canvas

ParupaintCanvasStrokeObject::ParupaintCanvasStrokeObject(ParupaintCanvasObject* canvas)
{
	this->canvasRectangle = QRectF(0, 0, canvas->GetWidth(), canvas->GetHeight());
	this->setZValue(0);
}


QRectF ParupaintCanvasStrokeObject::boundingRect() const
{
	return this->canvasRectangle;
}


void ParupaintCanvasStrokeObject::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
	if(strokes.isEmpty()) return;

	QRect exposed = option->exposedRect.adjusted(-1, -1, 1, 1).toAlignedRect();
	QPointF pos1 = strokes.first()->GetPosition();


	foreach(auto *i, strokes){
		QPen pen(i->GetColor(), i->GetWidth(), Qt::SolidLine, Qt::RoundCap);
		
		painter->save();
		
		painter->setClipRect(exposed);
		painter->setPen(pen);
		painter->drawLine(QLineF(pos1, i->GetPosition()));

		painter->restore();

		pos1 = i->GetPosition();
	}
}




