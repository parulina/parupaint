
#include <QKeyEvent>
#include <QEvent>
#include <QSettings>
#include <QShortcut>
#include <QTimer>

#include "parupaintWindow.h"

#include "stroke/parupaintStrokeStep.h"
#include "parupaintBrush.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"
#include "parupaintCanvasObject.h"

#include "panvas/parupaintLayer.h"
#include "panvas/parupaintFrame.h"

#include "overlay/parupaintChat.h"
#include "overlay/parupaintFlayer.h"
#include "overlay/parupaintFlayerList.h" // for SIGNAL
#include "overlay/parupaintColorPicker.h"
#include "overlay/parupaintUserList.h"
#include "overlay/parupaintInfoBar.h"


#include <QDebug>

ParupaintWindow::ParupaintWindow() : QMainWindow(), 
	// overlay keys
	OverlayKeyShow(Qt::Key_Tab), OverlayKeyHide(Qt::Key_Tab + Qt::SHIFT), 
	OverlayButtonDown(false), 
	// general keys
	CanvasKeyNextLayer(Qt::Key_D), CanvasKeyPreviousLayer(Qt::Key_S),
	CanvasKeyNextFrame(Qt::Key_F), CanvasKeyPreviousFrame(Qt::Key_A),
	//internal stuff?
	OverlayState(OVERLAY_STATUS_HIDDEN)

{



	auto * view = new ParupaintCanvasView(this);
	connect(view, SIGNAL(PenDraw(QPointF, ParupaintBrush*)), this, SLOT(PenDraw(QPointF, ParupaintBrush*)));
	connect(view, SIGNAL(PenDrawStart(ParupaintBrush*)), this, SLOT(PenDrawStart(ParupaintBrush*)));
	connect(view, SIGNAL(PenDrawStop(ParupaintBrush*)), this, SLOT(PenDrawStop(ParupaintBrush*)));
	setCentralWidget(view);

	canvas = new ParupaintCanvasPool(view);

	view->SetCanvas(canvas);
	canvas->GetCanvas()->AddLayers(1, 2); // add 2 layers at pos 1
	canvas->GetCanvas()->GetLayer(1)->SetFrames(10);
	for(auto i = 0; i < 10; i++){
		ParupaintFrame * frame = canvas->GetCanvas()->GetLayer(1)->GetFrame(i);
		frame->DrawStep(canvas->GetCanvas()->GetWidth() * (float(i)/10.0), 0, 
				canvas->GetCanvas()->GetWidth(), canvas->GetCanvas()->GetHeight(),
				i, Qt::black);
	}
	// canvas->GetCanvas() returns Panvas.

	chat =	  new ParupaintChat(this);
	flayer =  new ParupaintFlayer(this);
	picker =  new ParupaintColorPicker(this);
	infobar = new ParupaintInfoBar(this);
	
	connect(canvas->GetCanvas(), SIGNAL(CurrentSignal(int, int)), this, SLOT(ChangedFrame(int, int)));
	connect(flayer->GetList(), SIGNAL(clickFrame(int, int)), this, SLOT(SelectFrame(int, int)));


	OverlayKeyShow = QKeySequence(Qt::Key_Tab);
	OverlayKeyHide = QKeySequence(Qt::Key_Tab + Qt::SHIFT);


	OverlayButtonTimer = new QTimer(this);
	OverlayButtonTimer->setSingleShot(true);
	connect(OverlayButtonTimer, SIGNAL(timeout()), this, SLOT(TabTimeout()));

	OverlayTimer = new QTimer(this);
	OverlayTimer->setSingleShot(true);
	connect(OverlayTimer, SIGNAL(timeout()), this, SLOT(OverlayTimeout()));


	QShortcut * TabKey = new QShortcut(OverlayKeyShow, this);
	QShortcut * TabKeyShift = new QShortcut(OverlayKeyHide, this);
	connect(TabKey, SIGNAL(activated()), this, SLOT(OverlayKey()));
	connect(TabKeyShift, SIGNAL(activated()), this, SLOT(OverlayKey()));


	QShortcut * NextFrameKey = 	new QShortcut(CanvasKeyNextFrame, this);
	QShortcut * PreviousFrameKey = 	new QShortcut(CanvasKeyPreviousFrame, this);
	QShortcut * NextLayerKey = 	new QShortcut(CanvasKeyNextLayer, this);
	QShortcut * PreviousLayerKey =	new QShortcut(CanvasKeyPreviousLayer, this);

	connect(NextLayerKey, SIGNAL(activated()), this, SLOT(CanvasChangeKey()));
	connect(PreviousLayerKey, SIGNAL(activated()), this, SLOT(CanvasChangeKey()));
	connect(NextFrameKey, SIGNAL(activated()), this, SLOT(CanvasChangeKey()));
	connect(PreviousFrameKey, SIGNAL(activated()), this, SLOT(CanvasChangeKey()));


	UpdateTitle();
	flayer->UpdateFromCanvas(canvas->GetCanvas());
	
	QSettings cfg;
	restoreGeometry(cfg.value("mainWindowGeometry").toByteArray());
	restoreState(cfg.value("mainWindowState").toByteArray());

	show();
}


void ParupaintWindow::PenDrawStart(ParupaintBrush* brush){
	canvas->NewBrushStroke(brush);
}

void ParupaintWindow::PenDraw(QPointF pos, ParupaintBrush* brush){
	auto *stroke = brush->GetCurrentStroke();
	if(stroke != nullptr){
		stroke->AddStroke(new ParupaintStrokeStep(*brush));
	}
}

void ParupaintWindow::PenDrawStop(ParupaintBrush* brush){
	canvas->EndBrushStroke(brush);
}




void ParupaintWindow::ChangedFrame(int l, int f)
{
	flayer->SetMarkedLayerFrame(l, f);
}

void ParupaintWindow::SelectFrame(int l, int f)
{
	canvas->GetCanvas()->SetLayerFrame(l, f);
	canvas->UpdateView();
}

void ParupaintWindow::TabTimeout()
{
	OverlayButtonDown = false;
}

void ParupaintWindow::OverlayTimeout()
{
	HideOverlay();
}

void ParupaintWindow::OverlayKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	
	if(seq == OverlayKeyShow){

		if(!OverlayButtonDown) {
			OverlayButtonTimer->start(100);
			OverlayButtonDown = true;
			OverlayState = OVERLAY_STATUS_SHOWN_SMALL;

			ShowOverlay(false);

		} else {
			OverlayButtonTimer->stop();
			OverlayState = OVERLAY_STATUS_SHOWN_NORMAL;
			ShowOverlay(true);

		}

	} else if(seq == OverlayKeyHide){
		OverlayButtonDown = false;
		OverlayState = OVERLAY_STATUS_HIDDEN;
		HideOverlay();
	}
}

void ParupaintWindow::CanvasChangeKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	int ll = 0, ff = 0;
	if(seq == CanvasKeyNextFrame){
		ff ++;
	} else if(seq == CanvasKeyPreviousFrame){
		ff --;

	} else if(seq == CanvasKeyNextLayer){
		ll++;
	} else if(seq == CanvasKeyPreviousLayer){
		ll--;
	}

	canvas->GetCanvas()->AddLayerFrame(ll, ff);
	canvas->UpdateView();
}

void ParupaintWindow::ShowOverlay(bool permanent)
{
	if(!permanent) {
		OverlayTimer->start(1000);
	} else {
		OverlayTimer->stop();
	}
	chat->show();
	flayer->show();
	picker->show();

	UpdateOverlay();
}

void ParupaintWindow::HideOverlay()
{
	chat->hide();
	flayer->hide();
	picker->hide();
	UpdateOverlay();
}
void ParupaintWindow::UpdateOverlay()
{
	auto topbar = OverlayState == OVERLAY_STATUS_SHOWN_NORMAL ? 80 : 25;
	infobar->resize(this->width(), topbar);

	auto w1 = this->width() - chat->width();
	chat->move(w1, topbar);
	picker->move(0, topbar);

	auto h2 = this->height() - flayer->height();
	flayer->move(0, h2);
	flayer->resize(this->width(), flayer->height());
}

void ParupaintWindow::closeEvent(QCloseEvent *)
{
	QSettings cfg;
	cfg.setValue("mainWindowGeometry", saveGeometry());
	cfg.setValue("mainWindowState", saveState());
}


void ParupaintWindow::resizeEvent(QResizeEvent* event)
{
	UpdateOverlay();
	QMainWindow::resizeEvent(event);
}


void ParupaintWindow::UpdateTitle()
{
	setWindowTitle(QString("parupaint"));
}

