
#include "core/parupaintStrokeStep.h"
#include "parupaintCanvasStrokeObject.h"

#include "parupaintCanvasPool.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QLineF>

ParupaintCanvasStrokeObject::~ParupaintCanvasStrokeObject()
{
}

ParupaintCanvasStrokeObject::ParupaintCanvasStrokeObject()
{
}

ParupaintCanvasStrokeObject::ParupaintCanvasStrokeObject(QRectF limit) : ParupaintCanvasStrokeObject()
{
	SetRegionLimit(limit);
}

void ParupaintCanvasStrokeObject::SetRegionLimit(QRectF rect)
{
	region_limit = rect;
	QPixmap pix(region_limit.width(), region_limit.height());
	pix.fill(Qt::transparent);
	if(!this->pixmap().isNull()) pix = this->pixmap().copy(region_limit.toRect());
	
	this->setPixmap(pix);
}

void ParupaintCanvasStrokeObject::AddStroke(ParupaintStrokeStep *ss)
{
	if(this->pixmap().isNull()) return;

	auto pos1 = ss->GetPosition();
	const auto pos2 = pos1;
	if(!strokes.isEmpty()) pos1 = strokes.last()->GetPosition();
	ParupaintStroke::AddStroke(ss);



	QPixmap pix(this->pixmap());
 	QPainter paint(&pix);

	QPen pen = ss->ToPen();
	if(pen.color().alpha() == 0) pen.setBrush(Qt::red);
	paint.setPen(pen);
	paint.drawLine(pos1.x(), pos1.y(), pos2.x(), pos2.y());

 	this->setPixmap(pix);
}
