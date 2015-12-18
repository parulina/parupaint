#include "parupaintFlayer.h"

#include <QDebug>
#include <QRadioButton>
#include <QLabel>
#include <QScrollBar>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QGridLayout>

#include "../parupaintVisualCanvas.h"
#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"
#include "../widget/parupaintScrollBar.h"

ParupaintFlayerList::ParupaintFlayerList(QWidget * parent) : QFrame(parent)
{
	this->setAutoFillBackground(false);
	this->setContentsMargins(0, 0, 0, 0);

	QVBoxLayout * layout = new QVBoxLayout;
		layout->setAlignment(Qt::AlignTop);
		layout->setSpacing(1);
		layout->setMargin(0);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSizeConstraint(QLayout::SetFixedSize);
	this->setLayout(layout);
}


ParupaintFlayer::ParupaintFlayer(QWidget * parent) : QScrollArea(parent)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setAutoFillBackground(false);
	this->setContentsMargins(4, 4, 4, 4);

	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));

	this->setWidgetResizable(true);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	this->setWidget((layers = new ParupaintFlayerList));
}

void ParupaintFlayer::canvas_content_update()
{
	ParupaintPanvas * canvas = qobject_cast<ParupaintPanvas*>(sender());
	if(canvas){
		this->updateFromCanvas(canvas);
	}
}
void ParupaintFlayer::current_lf_update()
{
	ParupaintVisualCanvas * canvas = qobject_cast<ParupaintVisualCanvas*>(sender());
	if(canvas){
		this->setHighlightLayerFrame(canvas->currentLayer(), canvas->currentFrame());
	}
}

void ParupaintFlayer::frame_click()
{
	ParupaintFlayerFrame * frame = qobject_cast<ParupaintFlayerFrame*>(sender());
	if(frame){
		this->clearHighlight();
		frame->setChecked(true);

		emit onHighlightChange(frame->layer, frame->frame);
	}
}

void ParupaintFlayer::clearHighlight()
{
	QVBoxLayout * layout = qobject_cast<QVBoxLayout* >(layers->layout());
	for(int i = 0; i < layout->count(); i++){
		QLayoutItem * item = layout->itemAt(i);
		ParupaintFlayerLayer * layer = qobject_cast<ParupaintFlayerLayer*>(item->widget());
		if(layer){
			ParupaintFlayerFrame * frame;
			int ff = 0;
			while((frame = layer->frameAt(ff))){
				frame->setChecked(false);
				ff++;
			}
		}
	}
}
void ParupaintFlayer::updateFromCanvas(ParupaintPanvas* panvas)
{
	QVBoxLayout * layout = qobject_cast<QVBoxLayout* >(layers->layout());

	QLayoutItem * item;
	while((item = layout->takeAt(0)) != nullptr){
		delete item->widget();
		delete item;
	}

	for(int l = 0; l < panvas->layerCount(); l++){
		
		ParupaintFlayerLayer * flayer_layer = new ParupaintFlayerLayer;
		flayer_layer->setLayerVisible(panvas->layerAt(l)->visible());
		layout->addWidget(flayer_layer);

		for(int f = 0; f < panvas->layerAt(l)->frameCount(); f++){

			ParupaintFlayerFrame * frame = new ParupaintFlayerFrame;
			frame->layer = l; frame->frame = f;

			connect(frame, &ParupaintFlayerFrame::clicked, this, &ParupaintFlayer::frame_click);
			flayer_layer->addFrame(frame);
		}
	}
	this->setHighlightLayerFrame(0, 0);
}

void ParupaintFlayer::setHighlightLayerFrame(int l, int f)
{
	this->clearHighlight();

	QVBoxLayout * layout = qobject_cast<QVBoxLayout* >(layers->layout());
	QLayoutItem * item = layout->itemAt(l);
	if(!item) return;

	ParupaintFlayerLayer * layer = qobject_cast<ParupaintFlayerLayer*>(item->widget());
	if(layer){
		ParupaintFlayerFrame * frame = layer->frameAt(f);
		if(frame){
			frame->setChecked(true);
		}
	}
}

void ParupaintFlayer::mouseMoveEvent(QMouseEvent * event)
{
	if(old_pos.isNull()) old_pos = event->pos();

	if((event->modifiers() & Qt::ShiftModifier) ||
	   (event->buttons() & Qt::MiddleButton)){

		QPoint dif = (old_pos - event->pos());
		QScrollBar *ver = this->verticalScrollBar();
		QScrollBar *hor = this->horizontalScrollBar();
		hor->setSliderPosition(hor->sliderPosition() + dif.x());
		ver->setSliderPosition(ver->sliderPosition() + dif.y());
	}
	old_pos = event->pos();

	event->accept();
	QScrollArea::mouseMoveEvent(event);
}

bool ParupaintFlayer::event(QEvent * event)
{
	if(event->type() == QEvent::LayoutRequest){

		int height = layers->height() + 1;
		if(height > 200) height = 200;
		this->setFixedHeight(height);
	}
	return QScrollArea::event(event);
}

QSize ParupaintFlayer::minimumSizeHint() const
{
	return QSize(200, 20);
}
