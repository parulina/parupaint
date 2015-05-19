
#include "stroke/parupaintStrokeStep.h"
#include "parupaintCanvasStrokeObject.h"

#include "parupaintCanvasPool.h"

#include <QStyleOptionGraphicsItem>
#include <QPainter>
#include <QRectF>
#include <QLineF>

ParupaintCanvasStrokeObject::ParupaintCanvasStrokeObject(QRectF limit)
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
	if(!strokes.isEmpty()) pos1 = strokes.last()->GetPosition();
	ParupaintStroke::AddStroke(ss);

	
	QPixmap pix(this->pixmap());
 	QPainter paint(&pix);

	paint.setPen(QPen(ss->GetColor(), ss->GetWidth() * ss->GetPressure(), Qt::SolidLine, Qt::RoundCap));
	paint.drawLine(QLineF(pos1, ss->GetPosition()));

 	this->setPixmap(pix);
}
