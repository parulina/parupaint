
#include <QKeyEvent>
#include <QEvent>
#include <QSettings>
#include <QShortcut>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>

#include "parupaintWindow.h"
#include "core/parupaintPanvasReader.h"

#include "core/parupaintStrokeStep.h"
#include "core/parupaintBrush.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"
#include "parupaintCanvasObject.h"
#include "parupaintCanvasBrush.h"

#include "core/parupaintLayer.h"
#include "core/parupaintFrame.h"

#include "overlay/parupaintChat.h"
#include "overlay/parupaintFlayer.h"
#include "overlay/parupaintFlayerList.h" // for SIGNAL
#include "overlay/parupaintColorPicker.h"
#include "overlay/parupaintUserList.h"
#include "overlay/parupaintInfoBar.h"

#include "net/parupaintClientInstance.h"

#include <QDebug>

ParupaintWindow::ParupaintWindow() : QMainWindow(), 
	// overlay keys
	OverlayKeyShow(Qt::Key_Tab), OverlayKeyHide(Qt::Key_Tab + Qt::SHIFT), 
	OverlayButtonDown(false), 
	// general keys
	CanvasKeySquash(Qt::Key_Space),
	CanvasKeyNextLayer(Qt::Key_D), CanvasKeyPreviousLayer(Qt::Key_S),
	CanvasKeyNextFrame(Qt::Key_F), CanvasKeyPreviousFrame(Qt::Key_A),
	// network keys
	CanvasKeyReload(Qt::Key_R + Qt::SHIFT), CanvasKeyQuicksave(Qt::Key_K + Qt::ALT),
	CanvasKeyOpen(Qt::Key_O + Qt::ALT), CanvasKeySaveProject(Qt::Key_L + Qt::ALT),
	// brush keys
	BrushKeyUndo(Qt::Key_Z), BrushKeyRedo(Qt::SHIFT + Qt::Key_Z),
	BrushKeySwitchBrush(Qt::Key_E),
	//internal stuff?
	OverlayState(OVERLAY_STATUS_HIDDEN)

{
	view = new ParupaintCanvasView(this);
	view->SetCurrentBrush(glass.GetCurrentBrush());

	connect(view, &ParupaintCanvasView::CursorChange, this, &ParupaintWindow::CursorChange);
	connect(view, &ParupaintCanvasView::PenMove, this, &ParupaintWindow::PenMove);
	connect(view, &ParupaintCanvasView::PenDrawStart, this, &ParupaintWindow::PenDrawStart);
	connect(view, &ParupaintCanvasView::PenDrawStop, this, &ParupaintWindow::PenDrawStop);

	setCentralWidget(view);

	pool = new ParupaintCanvasPool(view);
	view->SetCanvas(pool);

	client = new ParupaintClientInstance(pool, QUrl("ws://localhost:1108"), this);


	chat =	  new ParupaintChat(this);
	flayer =  new ParupaintFlayer(this);
	picker =  new ParupaintColorPicker(this);
	infobar = new ParupaintInfoBar(this);
	
	connect(pool, SIGNAL(UpdateView()), this, SLOT(ViewUpdate()));
	connect(pool->GetCanvas(), SIGNAL(CurrentSignal(int, int)), this, SLOT(ChangedFrame(int, int)));
	connect(flayer->GetList(), SIGNAL(clickFrame(int, int)), this, SLOT(SelectFrame(int, int)));


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

	QShortcut * SwitchKey = new QShortcut(BrushKeySwitchBrush, this);
	QShortcut * UndoKey = 	new QShortcut(BrushKeyUndo, this);
	QShortcut * RedoKey = 	new QShortcut(BrushKeyRedo, this);

	connect(SwitchKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);
	connect(UndoKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);
	connect(RedoKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);

	// Network keys
	QShortcut * ReloadKey = new QShortcut(CanvasKeyReload, this);
	connect(ReloadKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	QShortcut * QuicksaveKey = new QShortcut(CanvasKeyQuicksave, this);
	connect(QuicksaveKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	QShortcut * OpenKey = new QShortcut(CanvasKeyOpen, this);
	connect(OpenKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	QShortcut * SaveProjectKey = new QShortcut(CanvasKeySaveProject, this);
	connect(SaveProjectKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);


	UpdateTitle();
	
	QSettings cfg;
	restoreGeometry(cfg.value("mainWindowGeometry").toByteArray());
	restoreState(cfg.value("mainWindowState").toByteArray());

	show();
}

void ParupaintWindow::CursorChange(ParupaintBrush* brush)
{
	// when the cursor changes. important to update brushglass
	*glass.GetCurrentBrush() = *brush;
}

ParupaintCanvasPool * ParupaintWindow::GetCanvasPool()
{
	return pool;
}

void ParupaintWindow::PenDrawStart(ParupaintBrush* brush){
	client->SendBrushUpdate(brush);
// 	pool->NewBrushStroke(brush);
}

void ParupaintWindow::PenMove(ParupaintBrush* brush){
	client->SendBrushUpdate(brush);

// 	auto *stroke = brush->GetCurrentStroke();
// 	if(stroke != nullptr){
// 		stroke->AddStroke(new ParupaintStrokeStep(*brush));
// 	}
}

void ParupaintWindow::PenDrawStop(ParupaintBrush* brush){
	client->SendBrushUpdate(brush);
// 	pool->EndBrushStroke(brush);
}


void ParupaintWindow::ViewUpdate()
{
	auto brush = glass.GetCurrentBrush();
	flayer->UpdateFromCanvas(pool->GetCanvas());
	flayer->SetMarkedLayerFrame(brush->GetLayer(), brush->GetFrame());
}

void ParupaintWindow::ChangedFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	pool->UpdateView();
	flayer->SetMarkedLayerFrame(l, f);
}

void ParupaintWindow::SelectFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	client->SendLayerFrame(brush);
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
			OverlayButtonTimer->start(200);
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

void ParupaintWindow::NetworkKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	if(seq == CanvasKeyReload){
		client->ReloadImage();

	} else if(seq == CanvasKeyQuicksave){
		client->SaveCanvas("test.png");
	
	} else if(seq == CanvasKeySaveProject) {
		client->LoadCanvas("animushin.tar.gz");

	} else if(seq == CanvasKeyOpen) {

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

	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(brush->GetLayer() + ll);
	brush->SetFrame(brush->GetFrame() + ff);
	client->SendLayerFrame(brush);

}

void ParupaintWindow::BrushKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();
	
	auto brush = glass.GetCurrentBrush();

	if(seq == BrushKeyUndo){
		pool->UndoBrushStroke(brush);
	} else if(seq == BrushKeyRedo) {
		pool->RedoBrushStroke(brush);

	} else if(seq == BrushKeySwitchBrush) {

		glass.ToggleBrush(0, 1);
		auto brush2 = glass.GetCurrentBrush();
		brush2->SetDrawing(brush->IsDrawing());

		view->SetCurrentBrush(brush2);
		brush2->SetPosition(brush->GetPosition());
	}

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
	auto visible = OverlayState == OVERLAY_STATUS_SHOWN_NORMAL ? infobar->height() : 35;
	infobar->move(0, visible - infobar->height());
	infobar->resize(this->width(), infobar->height());

	auto w1 = this->width() - chat->width();
	chat->move(w1, visible);
	picker->move(0, visible);

	auto h2 = this->height() - flayer->height();
	flayer->move(0, h2);
	flayer->resize(this->width(), flayer->height());
}

void ParupaintWindow::keyPressEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space && !event->isAutoRepeat()){
		if(OverlayButtonDown){
			auto brush = glass.GetCurrentBrush();
			if(event->modifiers() & Qt::ControlModifier){
				pool->ClearBrushStrokes(brush);
			} else {
				pool->SquashBrushStrokes(brush);
			}
		}
	}
	return QMainWindow::keyPressEvent(event);
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

