
#include "parupaintFlayerLayer.h"
#include "parupaintFlayerFrame.h"

#include <QHBoxLayout>
#include <QStyleOption>
#include <QPainter>

#include <QDebug>

ParupaintFlayerLayer::ParupaintFlayerLayer(QWidget * parent) : QWidget(parent), Index(0)
{
	this->setFocusPolicy(Qt::NoFocus);
	this->setObjectName("FlayerLayer");
	this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	this->setMinimumHeight(15);

	box = new QHBoxLayout(this);
	box->setSpacing(1);
	box->setMargin(0);
	box->setAlignment(Qt::AlignLeft);
}

ParupaintFlayerFrame * ParupaintFlayerLayer::NewFrame()
{
	return new ParupaintFlayerFrame(this);
}

ParupaintFlayerFrame * ParupaintFlayerLayer::AddFrame(int index) { return AddFrame(index, NewFrame()); }
ParupaintFlayerFrame * ParupaintFlayerLayer::AddFrame(int index, ParupaintFlayerFrame * frame)
{
	connect(frame, SIGNAL(clicked()), this, SLOT(FrameChange()));
	Frames.insert(index, frame);
	box->insertWidget(index, frame);
	return frame;	
}

void ParupaintFlayerLayer::RemoveFrame(int index)
{
	GetFrame(index)->deleteLater();
	Frames.removeAt(index);
}

void ParupaintFlayerLayer::MoveFrame(int index, int offset)
{
	if(index+offset < 0) 			offset = 0 - index;
	if(index+offset >= Frames.length()) 	offset = (Frames.length()-1) - index;

	box->removeWidget(GetFrame(index));
	box->insertWidget(index+offset, GetFrame(index));
	Frames.move(index, index+offset);
}

ParupaintFlayerFrame* ParupaintFlayerLayer::GetFrame(int index)
{
	if(Frames.isEmpty() || index >= Frames.size()) return nullptr;
	return Frames.at(index);
}

void ParupaintFlayerLayer::Clear()
{
	foreach(auto *i, Frames){
		i->deleteLater();
	}
	Frames.clear();
}
void ParupaintFlayerLayer::ClearChecked()
{
	foreach(auto *i, Frames){
		i->setChecked(false);
	}
}
void ParupaintFlayerLayer::FrameChange()
{
	ParupaintFlayerFrame* button = static_cast<ParupaintFlayerFrame*>(sender());
	this->ClearChecked();
	emit frameCheck(button);
	button->setChecked(true);
}
