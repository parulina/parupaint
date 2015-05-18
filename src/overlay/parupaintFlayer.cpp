
#include "parupaintFlayer.h"
#include "parupaintFlayerList.h"


#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"
#include "../parupaintCanvasObject.h"
#include "../panvas/parupaintLayer.h"
#include "../panvas/parupaintFrame.h"

#include <QScrollBar>
#include <QMouseEvent>
#include <QKeyEvent>

#include <QDebug>

ParupaintFlayer::ParupaintFlayer(QWidget * parent) : QScrollArea(parent)
{
	this->setObjectName("Flayer");

	this->resize(0, 200);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setWidgetResizable(true);

	list = new ParupaintFlayerList(this);
	this->setWidget(list);

	viewport()->setMouseTracking(true);
}

ParupaintFlayerList * ParupaintFlayer::GetList()
{
	return list;
}


void ParupaintFlayer::UpdateFromCanvas(ParupaintCanvasObject * canvas)
{
	// clear everything
	list->Clear();

	ParupaintPanvas * panvas = static_cast<ParupaintPanvas*>(canvas);
	for(auto l = 0; l < panvas->GetNumLayers(); l++){
		auto * layer = panvas->GetLayer(l);
		auto * nlayer = list->AddLayer(l);

		nlayer->Index = l;
		for(auto f = 0; f < layer->GetNumFrames(); f++){
			auto * frame = layer->GetFrame(f);
			auto * nframe = nlayer->AddFrame(f);
			nframe->Index = f;
			nframe->Extended = frame->IsExtended();
		}
	}
}





void ParupaintFlayer::mousePressEvent(QMouseEvent * event)
{
	if(event->buttons() & Qt::RightButton) {
		FlayerState = FLAYER_STATUS_MOVING;
	}
	OldPosition = event->pos();
	QScrollArea::mousePressEvent(event);
}

void ParupaintFlayer::mouseReleaseEvent(QMouseEvent * event)
{
	FlayerState = FLAYER_STATUS_IDLE;
	OldPosition = event->pos();
	QScrollArea::mouseReleaseEvent(event);
}

void ParupaintFlayer::mouseMoveEvent(QMouseEvent * event)
{
	if(FlayerState == FLAYER_STATUS_MOVING){
		QScrollBar *ver = this->verticalScrollBar();
		QScrollBar *hor = this->horizontalScrollBar();
		auto dif = OldPosition - event->pos();
		hor->setSliderPosition(hor->sliderPosition() + dif.x());
		ver->setSliderPosition(ver->sliderPosition() + dif.y());
	}
	OldPosition = event->pos();
	QScrollArea::mouseMoveEvent(event);
}

void ParupaintFlayer::keyPressEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space && !event->isAutoRepeat() && 
			FlayerState != FLAYER_STATUS_MOVING){
		FlayerState = FLAYER_STATUS_MOVING;

	}
	QScrollArea::keyPressEvent(event);
}

void ParupaintFlayer::keyReleaseEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space && !event->isAutoRepeat() &&
			FlayerState != FLAYER_STATUS_IDLE){
		FlayerState = FLAYER_STATUS_IDLE;
	}
	QScrollArea::keyReleaseEvent(event);
}

void ParupaintFlayer::enterEvent(QEvent * event)
{
	viewport()->setMouseTracking(true);
	QScrollArea::enterEvent(event);
}
void ParupaintFlayer::leaveEvent(QEvent * event)
{
	viewport()->setMouseTracking(false);
	QScrollArea::leaveEvent(event);
}

