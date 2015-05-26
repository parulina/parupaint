
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

#include "parupaintConnectionDialog.h"
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
	CanvasKeyReload(Qt::Key_R + Qt::SHIFT), CanvasKeyQuicksave(Qt::Key_K + Qt::CTRL),
	CanvasKeyOpen(Qt::Key_O + Qt::CTRL), CanvasKeySaveProject(Qt::Key_L + Qt::CTRL),
	CanvasKeyPreview(Qt::Key_Q), CanvasKeyConnect(Qt::Key_I + Qt::CTRL),
	// brush keys
	BrushKeyUndo(Qt::Key_Z), BrushKeyRedo(Qt::SHIFT + Qt::Key_Z),
	BrushKeySwitchBrush(Qt::Key_E),
	//internal stuff?
	OverlayState(OVERLAY_STATUS_HIDDEN)

{
	this->setFocusPolicy(Qt::NoFocus);

	view = new ParupaintCanvasView(this);
	view->SetCurrentBrush(glass.GetCurrentBrush());

	connect(view, &ParupaintCanvasView::CursorChange, this, &ParupaintWindow::CursorChange);
	connect(view, &ParupaintCanvasView::PenMove, this, &ParupaintWindow::PenMove);
	connect(view, &ParupaintCanvasView::PenDrawStart, this, &ParupaintWindow::PenDrawStart);
	connect(view, &ParupaintCanvasView::PenDrawStop, this, &ParupaintWindow::PenDrawStop);

	setCentralWidget(view);

	pool = new ParupaintCanvasPool(view);
	view->SetCanvas(pool);

	client = new ParupaintClientInstance(pool, this);

	chat =	  new ParupaintChat(this);
	flayer =  new ParupaintFlayer(this);
	picker =  new ParupaintColorPicker(this);
	infobar = new ParupaintInfoBar(this);
	
	// when canvas is updated, frames/layers are added - do a view update that updates the flayer panel
	connect(pool, &ParupaintCanvasPool::UpdateCanvas, this, &ParupaintWindow::ViewUpdate);

	connect(pool->GetCanvas(), SIGNAL(CurrentSignal(int, int)), this, SLOT(ChangedFrame(int, int)));
	connect(flayer->GetList(), SIGNAL(clickFrame(int, int)), this, SLOT(SelectFrame(int, int)));


	OverlayTimer = new QTimer(this);
	OverlayTimer->setSingleShot(true);
	connect(OverlayTimer, SIGNAL(timeout()), this, SLOT(OverlayTimeout()));


	QShortcut * PreviewKey =	new QShortcut(CanvasKeyPreview, this);
	connect(PreviewKey, 		&QShortcut::activated, this, &ParupaintWindow::CanvasKey);

	QShortcut * NextFrameKey = 	new QShortcut(CanvasKeyNextFrame, this);
	QShortcut * PreviousFrameKey = 	new QShortcut(CanvasKeyPreviousFrame, this);
	QShortcut * NextLayerKey = 	new QShortcut(CanvasKeyNextLayer, this);
	QShortcut * PreviousLayerKey =	new QShortcut(CanvasKeyPreviousLayer, this);

	connect(NextLayerKey, 		&QShortcut::activated, this, &ParupaintWindow::CanvasChangeKey);
	connect(PreviousLayerKey,	&QShortcut::activated, this, &ParupaintWindow::CanvasChangeKey);
	connect(NextFrameKey, 		&QShortcut::activated, this, &ParupaintWindow::CanvasChangeKey);
	connect(PreviousFrameKey, 	&QShortcut::activated, this, &ParupaintWindow::CanvasChangeKey);

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
	QShortcut * ConnectKey = new QShortcut(CanvasKeyConnect, this);
	connect(ConnectKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);

	connection_dialog = new ParupaintConnectionDialog(this);
	connect(connection_dialog, &ParupaintConnectionDialog::ConnectSignal, this, &ParupaintWindow::Connect);
	

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
}

void ParupaintWindow::ChangedFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	flayer->SetMarkedLayerFrame(l, f);
}

void ParupaintWindow::SelectFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	client->SendLayerFrame(l, f);
}

void ParupaintWindow::OverlayTimeout()
{
	HideOverlay();
}


void ParupaintWindow::NetworkKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	if(seq == CanvasKeyReload){
		client->ReloadImage();

	} else if(seq == CanvasKeyQuicksave){
		client->SaveCanvas(".png");
	
	} else if(seq == CanvasKeySaveProject) {
		client->LoadCanvas("animushin.tar.gz");

	} else if(seq == CanvasKeyConnect) {
		connection_dialog->show();

	} else if(seq == CanvasKeyOpen) {

	}
}

void ParupaintWindow::CanvasKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	if(seq == CanvasKeyPreview){
		pool->GetCanvas()->TogglePreview();
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
	if(!OverlayButtonDown) {
		// Do a local check for boundaries
		pool->GetCanvas()->AddLayerFrame(ll, ff);

		auto brush = glass.GetCurrentBrush();
		brush->SetLayer(pool->GetCanvas()->GetCurrentLayer());
		brush->SetFrame(pool->GetCanvas()->GetCurrentFrame());
		client->SendLayerFrame(pool->GetCanvas()->GetCurrentLayer(),
					pool->GetCanvas()->GetCurrentFrame());
	} else {
		
		client->SendLayerFrame(pool->GetCanvas()->GetCurrentLayer(),
						pool->GetCanvas()->GetCurrentFrame(),
						ll, ff);
	}

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

bool ParupaintWindow::focusNextPrevChild(bool b)
{
	if(connection_dialog->isVisible()) {
		return QMainWindow::focusNextPrevChild(b);
	}
	return false; // disable tab
}

void ParupaintWindow::keyReleaseEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Tab && !event->isAutoRepeat()){
		OverlayButtonDown = false;
		return;
	}
	return QMainWindow::keyReleaseEvent(event);
}

void ParupaintWindow::keyPressEvent(QKeyEvent * event)
{
	if(connection_dialog->isVisible()) {
		return QMainWindow::keyPressEvent(event);
	}

	if(event->key() == Qt::Key_F1 && !event->isAutoRepeat()){
		OverlayState = OVERLAY_STATUS_SHOWN_NORMAL;
		ShowOverlay(true);
	}
	if(event->key() == Qt::Key_Backtab) {
		OverlayState = OVERLAY_STATUS_HIDDEN;
		HideOverlay();
	}
	if(event->key() == Qt::Key_Tab){
	
		if(!event->isAutoRepeat()) {
			OverlayState = OVERLAY_STATUS_SHOWN_SMALL;
			ShowOverlay(false);

		} else {
			OverlayState = OVERLAY_STATUS_SHOWN_NORMAL;
			ShowOverlay(true);

		}
		OverlayButtonDown = true;
		return;
	}
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

void ParupaintWindow::Connect(QString url)
{
	client->Connect(url);
}
