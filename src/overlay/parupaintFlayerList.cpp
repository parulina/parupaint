
#include "parupaintFlayerList.h"
#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"

#include <QVBoxLayout>
#include <QDebug>


ParupaintFlayerList::ParupaintFlayerList(QWidget * parent) : QWidget(parent)
{
	this->setObjectName("FlayerList");
	box = new QVBoxLayout(this);
	box->setSizeConstraint(QLayout::SetMinimumSize);
	box->setAlignment(Qt::AlignTop);
	box->setSpacing(2);
	box->setMargin(2);

	this->setLayout(box);
}

ParupaintFlayerLayer * ParupaintFlayerList::NewLayer()
{
	return new ParupaintFlayerLayer(this);
}

ParupaintFlayerLayer * ParupaintFlayerList::AddLayer(int index) { return AddLayer(index, NewLayer()); }
ParupaintFlayerLayer *  ParupaintFlayerList::AddLayer(int index, ParupaintFlayerLayer * layer)
{
	connect(layer, SIGNAL(frameCheck(ParupaintFlayerFrame*)), this, SLOT(FrameChecked(ParupaintFlayerFrame*)));
	Layers.insert(index, layer);
	box->insertWidget(index, layer);
	return layer;	
}

void ParupaintFlayerList::RemoveLayer(int index)
{
	GetLayer(index)->deleteLater();
	Layers.removeAt(index);
}

void ParupaintFlayerList::MoveLayer(int index, int offset)
{
	if(index+offset < 0) 			offset = 0 - index;
	if(index+offset >= Layers.length()) 	offset = (Layers.length()-1) - index;

	box->removeWidget(GetLayer(index));
	box->insertWidget(index+offset, GetLayer(index));
	Layers.move(index, index+offset);
}

ParupaintFlayerLayer * ParupaintFlayerList::GetLayer(int index)
{
	if(Layers.isEmpty() || index >= Layers.size()) return nullptr;
	return Layers.at(index);
}

void ParupaintFlayerList::Clear()
{
	foreach(auto *i, Layers){
		i->deleteLater();
	}
	Layers.clear();
}


void ParupaintFlayerList::ClearAllChecked()
{
	foreach(auto *i, Layers){
		i->ClearChecked();
	}
}

void ParupaintFlayerList::FrameChecked(ParupaintFlayerFrame*b)
{
	ParupaintFlayerLayer* layer = static_cast<ParupaintFlayerLayer*>(sender());
	foreach(auto *i, Layers){
		if(i != layer) i->ClearChecked();
	}

	emit clickFrame(layer ? layer->Index : 0, b ? b->Index : 0);
}

