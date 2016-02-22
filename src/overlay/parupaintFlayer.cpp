#include "parupaintFlayer.h"

#include <QDebug>
#include <QRadioButton>
#include <QLabel>
#include <QScrollBar>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QGridLayout>
#include <QElapsedTimer>

#include "../parupaintVisualCanvas.h"
#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"
#include "../widget/parupaintScrollBar.h"

ParupaintFlayer::ParupaintFlayer(QWidget * parent) : QListWidget(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	this->setAutoFillBackground(false);
	this->setContentsMargins(4, 4, 4, 4);

	this->setVerticalScrollBar(new ParupaintScrollBar(Qt::Vertical, this));
	this->setHorizontalScrollBar(new ParupaintScrollBar(Qt::Horizontal, this));

	this->setSelectionMode(QAbstractItemView::SingleSelection);
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

// if layers and frames hasn't changed, just attributes
void ParupaintFlayer::reloadCanvasSlot()
{
	ParupaintPanvas * canvas = qobject_cast<ParupaintPanvas*>(sender());
	if(canvas){
		this->updateFromCanvas(canvas);
	}
}
// if layers and frames has changed
void ParupaintFlayer::updateCanvasSlot()
{
	qDebug() << "update canvas";
	ParupaintPanvas * canvas = qobject_cast<ParupaintPanvas*>(sender());
	if(canvas){
		for(int l = 0; l < canvas->layerCount(); l++){
			ParupaintLayer * layer = canvas->layerAt(l);
			if(!layer) break;

			ParupaintFlayerLayer * flayer = this->flayer(l);
			flayer->setLayerName(layer->name());
			flayer->setLayerMode(layer->mode());
			flayer->setLayerVisible(layer->visible());

			for(int f = 0; f < layer->frameCount(); f++){
				ParupaintFrame * frame = layer->frameAt(f);
				if(!frame) break;

				ParupaintFlayerFrame * flame = flayer->frameAt(f);
			}
		}
	}
}

ParupaintFlayerLayer *  ParupaintFlayer::flayer(int i)
{
	int il = (this->count() - 1) - i;
	return dynamic_cast<ParupaintFlayerLayer *>(this->itemWidget(this->item(il)));
}

void ParupaintFlayer::clearHighlight()
{
	for(int i = 0; i < this->count(); i++){
		ParupaintFlayerLayer * flayer = this->flayer(i);
		flayer->selectFrame(-1);
	}
}
void ParupaintFlayer::updateFromCanvas(ParupaintPanvas* panvas)
{
	QElapsedTimer debug_timer;
	debug_timer.start();

	while(this->count()){
		delete this->takeItem(0);
	}

	for(int l = 0; l < panvas->layerCount(); l++){
		ParupaintLayer * layer = panvas->layerAt(l);
		if(!layer) break;

		ParupaintFlayerLayer * flayer_layer = new ParupaintFlayerLayer(l);
		flayer_layer->setLayerVisible(layer->visible());
		flayer_layer->setLayerMode(layer->mode());
		flayer_layer->setLayerName(layer->name());

		connect(flayer_layer, &ParupaintFlayerLayer::visibleChanged, this, &ParupaintFlayer::onLayerVisibleChange);
		connect(flayer_layer, &ParupaintFlayerLayer::nameChanged, this, &ParupaintFlayer::onLayerNameChange);
		connect(flayer_layer, &ParupaintFlayerLayer::modeChanged, this, &ParupaintFlayer::onLayerModeChange);

		for(int f = 0; f < layer->frameCount(); f++){
			ParupaintFlayerFrame * frame = new ParupaintFlayerFrame(nullptr, l, f);
			if(layer) frame->setProperty("extended", layer->isFrameExtended(f));

			connect(frame, &ParupaintFlayerFrame::onLayerFrameClick, this, &ParupaintFlayer::onHighlightChange);
			flayer_layer->addFrame(frame);
		}

		QListWidgetItem * item = new QListWidgetItem;
		item->setSizeHint(flayer_layer->sizeHint());
		this->insertItem(0, item);
		this->setItemWidget(item, flayer_layer);
	}

	qDebug() << "Done:" << debug_timer.elapsed();
}

void ParupaintFlayer::setHighlightLayerFrame(int l, int f)
{
	this->clearHighlight();

	int il = (this->count() - 1) - l;
	QListWidgetItem * item = this->item(il);
	ParupaintFlayerLayer * flayer = this->flayer(l);
	if(flayer){
		this->setCurrentRow(il);
		flayer->selectFrame(f);
	}
}

void ParupaintFlayer::setLayerVisible(int l, bool visible)
{
	ParupaintFlayerLayer * flayer = this->flayer(l);
	if(flayer){
		flayer->setLayerVisible(visible);
	}
}
void ParupaintFlayer::setLayerName(int l, const QString & name)
{
	ParupaintFlayerLayer * flayer = this->flayer(l);
	if(flayer){
		flayer->setLayerName(name);
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
	QListWidget::mouseMoveEvent(event);
}
void ParupaintFlayer::mouseReleaseEvent(QMouseEvent * event)
{
	if(event && event->button() == Qt::MiddleButton){
		old_pos = QPoint();
	}
}

QSize ParupaintFlayer::minimumSizeHint() const
{
	return QSize(200, 20);
}
