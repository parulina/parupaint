
#include <QKeyEvent>
#include <QEvent>
#include <QSettings>
#include <QShortcut>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QMimeData>

#include "parupaintVersionCheck.h"

#include "parupaintWindow.h"
#include "core/parupaintPanvasWriter.h"

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
#include "parupaintFileDialog.h"
#include "parupaintNewDialog.h"
#include "parupaintSettingsDialog.h"

#include "core/parupaintFrameBrushOps.h"

#include "net/parupaintClientInstance.h"

#include <QDebug>

ParupaintWindow::~ParupaintWindow()
{
	delete client;
}

ParupaintWindow::ParupaintWindow() : QMainWindow(), 
	local_port(1108), old_brush_switch(0),
	// overlay keys
	OverlayKeyShow(Qt::Key_Tab), OverlayKeyHide(Qt::Key_Tab + Qt::SHIFT), 
	OverlayButtonDown(false), 
	// general keys
	CanvasKeySquash(Qt::Key_Space),
	CanvasKeyNextLayer(Qt::Key_D), CanvasKeyPreviousLayer(Qt::Key_S),
	CanvasKeyNextFrame(Qt::Key_F), CanvasKeyPreviousFrame(Qt::Key_A),
	// network keys
	CanvasKeySettings(Qt::Key_M), CanvasKeyReload(Qt::Key_R), 
	CanvasKeyOpen(Qt::Key_O + Qt::CTRL), CanvasKeyNew(Qt::Key_N),
	CanvasKeyQuicksave(Qt::Key_K + Qt::CTRL), CanvasKeySaveProject(Qt::Key_L + Qt::CTRL),
	CanvasKeyPreview(Qt::Key_G), CanvasKeyConnect(Qt::Key_I + Qt::CTRL),
	CanvasKeyChat(Qt::Key_Return),
	// brush keys
	BrushKeyUndo(Qt::Key_Z), BrushKeyRedo(Qt::SHIFT + Qt::Key_Z),
	BrushKeySwitchBrush(Qt::Key_E), BrushKeyPickColor(Qt::Key_R),
	BrushKeyToolKey1(Qt::Key_Q), BrushKeyToolKey2(Qt::Key_W),
	//internal stuff?
	OverlayState(OVERLAY_STATUS_HIDDEN)

{

	auto * version_check = new ParupaintVersionCheck();
	connect(version_check, &ParupaintVersionCheck::Response, this, &ParupaintWindow::VersionResponse);

	view = new ParupaintCanvasView(this);
	view->SetCurrentBrush(glass.GetCurrentBrush());
	this->setFocusProxy(view);
	this->setFocusPolicy(Qt::StrongFocus);

	connect(view, &ParupaintCanvasView::CursorChange, this, &ParupaintWindow::CursorChange);
	connect(view, &ParupaintCanvasView::PenMove, this, &ParupaintWindow::PenMove);
	connect(view, &ParupaintCanvasView::PenDrawStart, this, &ParupaintWindow::PenDrawStart);
	connect(view, &ParupaintCanvasView::PenDrawStop, this, &ParupaintWindow::PenDrawStop);
	connect(view, &ParupaintCanvasView::PenPointerType, this, &ParupaintWindow::PenPointerType);

	setCentralWidget(view);

	pool = new ParupaintCanvasPool(view);
	view->SetCanvas(pool);

	client = new ParupaintClientInstance(pool, this);
	connect(client, &ParupaintClientInstance::ChatMessageReceived, this, &ParupaintWindow::ChatMessageReceived);

	chat =	  new ParupaintChat(this);
	flayer =  new ParupaintFlayer(this);
	picker =  new ParupaintColorPicker(this);
	infobar = new ParupaintInfoBar(this);

	picker->SetColor(glass.GetCurrentBrush()->GetColor());
	
	// when canvas is updated, frames/layers are added - do a view update that updates the flayer panel
	connect(pool, &ParupaintCanvasPool::UpdateCanvas, this, &ParupaintWindow::ViewUpdate);

	connect(pool->GetCanvas(), SIGNAL(CurrentSignal(int, int)), this, SLOT(ChangedFrame(int, int)));
	connect(flayer->GetList(), SIGNAL(clickFrame(int, int)), this, SLOT(SelectFrame(int, int)));
	connect(chat, &ParupaintChat::Message, this, &ParupaintWindow::ChatMessage);
	connect(picker, &ParupaintColorPicker::ColorChange, this, &ParupaintWindow::ColorChange);


	OverlayTimer = new QTimer(this);
	OverlayTimer->setSingleShot(true);
	connect(OverlayTimer, SIGNAL(timeout()), this, SLOT(OverlayTimeout()));

	OverlayButtonTimer = new QTimer(this);
	OverlayButtonTimer->setSingleShot(true);
	connect(OverlayButtonTimer, &QTimer::timeout, this, &ParupaintWindow::ButtonTimeout);

	QShortcut * SwitchKey = new QShortcut(BrushKeySwitchBrush, this);
	QShortcut * UndoKey = 	new QShortcut(BrushKeyUndo, this);
	QShortcut * RedoKey = 	new QShortcut(BrushKeyRedo, this);

	connect(SwitchKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);
	connect(UndoKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);
	connect(RedoKey, &QShortcut::activated, this, &ParupaintWindow::BrushKey);

	// Network keys
	QShortcut * QuicksaveKey = new QShortcut(CanvasKeyQuicksave, this);
	QShortcut * OpenKey = new QShortcut(CanvasKeyOpen, this);
	QShortcut * SaveProjectKey = new QShortcut(CanvasKeySaveProject, this);
	QShortcut * ConnectKey = new QShortcut(CanvasKeyConnect, this);

	connect(QuicksaveKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	connect(OpenKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	connect(SaveProjectKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);
	connect(ConnectKey, &QShortcut::activated, this, &ParupaintWindow::NetworkKey);

	this->setAcceptDrops(true);

	UpdateTitle();
	
	QSettings cfg;
	restoreGeometry(cfg.value("mainWindowGeometry").toByteArray());
	restoreState(cfg.value("mainWindowState").toByteArray());

	show();
}

void ParupaintWindow::VersionResponse(bool update, QString body)
{
	qDebug() << (update ? "New update." : "No new update.");

	if(update) chat->AddMessage(body);
}

void ParupaintWindow::ColorChange(QColor col)
{
	auto * brush = glass.GetCurrentBrush();
	brush->SetColor(col); // Save it
	view->UpdateCurrentBrush(brush); // show it
}

void ParupaintWindow::ChatMessageReceived(QString name, QString msg)
{
	chat->show();
	chat->AddMessage(msg, name);
}

void ParupaintWindow::ChatMessage(QString str)
{
	if(str[0] == '/'){
		QString cmd = str;
		QString params = "";
		if(str.indexOf(" ") != -1){
			cmd = str.section(" ", 0, 0);
			params = str.section(" ", 1);
		}
		cmd = cmd.mid(1);
		return Command(cmd, params);
	}
	client->SendChat(str);
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
	if(brush->GetToolType() == 1){
		//special
		glass.GetCurrentBrush()->SetDrawing(false);
		view->UpdateCurrentBrush(glass.GetCurrentBrush());
		return;
	}
	if(client->GetDrawMode() != DRAW_MODE_DIRECT){
		pool->NewBrushStroke(brush);
	}
}

void ParupaintWindow::PenMove(ParupaintBrush* brush){
	auto * cbrush = glass.GetCurrentBrush();

	if(brush->IsDrawing() && client->GetDrawMode() == DRAW_MODE_DIRECT){
		auto 	old_x = cbrush->GetPosition().x(),
			old_y = cbrush->GetPosition().y(),
			x = brush->GetPosition().x(),
			y = brush->GetPosition().y();

		QRect r = ParupaintFrameBrushOps::stroke(pool->GetCanvas(), old_x, old_y, x, y, brush);
		pool->GetCanvas()->RedrawCache(r);

	}
	cbrush->SetPosition(brush->GetPosition());

	if(brush->GetToolType() == 1) return;
	client->SendBrushUpdate(brush);
	if(brush->GetToolType() != 0) return;

	if(client->GetDrawMode() != DRAW_MODE_DIRECT){
		auto *stroke = brush->GetCurrentStroke();
		if(stroke != nullptr){
			stroke->AddStroke(new ParupaintStrokeStep(*brush));
		}
	}
}

void ParupaintWindow::PenDrawStop(ParupaintBrush* brush){

	if(brush->GetToolType() == 1) return;

	client->SendBrushUpdate(brush);
	if(client->GetDrawMode() != DRAW_MODE_DIRECT){
		pool->EndBrushStroke(brush);
		pool->ClearBrushStrokes(brush);
	}
}
void ParupaintWindow::PenPointerType(QTabletEvent::PointerType, QTabletEvent::PointerType nuw)
{
	if(nuw == QTabletEvent::Eraser) {
		glass.SetBrush(1);
		view->SetCurrentBrush(glass.GetCurrentBrush());
		picker->SetColor(glass.GetCurrentBrush()->GetColor());

	} else if(nuw == QTabletEvent::Pen) {
		glass.SetBrush(0);
		view->SetCurrentBrush(glass.GetCurrentBrush());
		picker->SetColor(glass.GetCurrentBrush()->GetColor());
	}
}


void ParupaintWindow::ViewUpdate()
{
	flayer->UpdateFromCanvas(pool->GetCanvas());
	infobar->SetCurrentDimensions(pool->GetCanvas()->GetWidth(), pool->GetCanvas()->GetHeight());
}

void ParupaintWindow::ChangedFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	flayer->SetMarkedLayerFrame(l, f);
	infobar->SetCurrentLayerFrame(l+1, f+1);
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
	OverlayState = OVERLAY_STATUS_HIDDEN;
	HideOverlay();
}

void ParupaintWindow::ButtonTimeout()
{
	OverlayButtonDown = false;
}


void ParupaintWindow::NetworkKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();

	if(seq == CanvasKeyQuicksave){
		QString saved = this->SaveAs(".png");
		QString filelink = QDir(this->GetSaveDirectory()).filePath(saved);
		chat->AddMessage("Quicksaved to '<a href=\""+filelink+"\">"+saved+"</a>'.");
	
	} else if(seq == CanvasKeySaveProject) {
		QSettings cfg;
		ParupaintFileDialog * dlg = new ParupaintFileDialog(this, ".png", "save as...");
		dlg->show();
		connect(dlg, &ParupaintFileDialog::EnterSignal, [=](QString str){
			QString saved = this->SaveAs(str);
			QString filelink = QDir(this->GetSaveDirectory()).filePath(saved);
			chat->AddMessage("Saved as '<a href=\"file:///"+filelink+"\">"+saved+"</a>'.");
			delete dlg;
		});

	} else if(seq == CanvasKeyConnect) {
		ParupaintConnectionDialog * dlg = new ParupaintConnectionDialog(this);
		dlg->show();
		dlg->setEnabled(true);

		connect(dlg, &ParupaintConnectionDialog::ConnectSignal, [=](QString addr){
			// i think i should use QDialog->done(); internally instead of deleting it...
			this->Connect(addr);
			delete dlg;
		});
		connect(dlg, &ParupaintConnectionDialog::DisconnectSignal, [=]{
			this->Disconnect();
			delete dlg;
		});

	} else if(seq == CanvasKeyOpen) {

		QSettings cfg;
		auto lastopen = cfg.value("net/lastopen").toString();
		ParupaintFileDialog * dlg = new ParupaintFileDialog(this, lastopen, "open...");
		dlg->show();

		connect(dlg, &ParupaintFileDialog::EnterSignal, [=](QString str){
			this->Open(str);
			delete dlg;
		});
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

		// Todo: move to keyPress and make shift mod force eraser mode
		pool->EndBrushStroke(brush);

		glass.ToggleBrush(old_brush_switch, 1);
		view->SetCurrentBrush(glass.GetCurrentBrush());
		picker->SetColor(glass.GetCurrentBrush()->GetColor());

		old_brush_switch = 0;
	}

}

void ParupaintWindow::ShowOverlay(bool permanent)
{
	if(!permanent) {
		OverlayTimer->start(1400);
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
	auto visible = OverlayState == OVERLAY_STATUS_SHOWN_NORMAL ? infobar->height() : 30;
	infobar->move(0, visible - infobar->height());
	infobar->resize(this->width(), infobar->height());

	auto spad = 15; // scroll padding
	auto w1 = this->width() - chat->width() - spad;
	chat->move(w1, visible);
	picker->move(0, visible);

	auto h2 = this->height() - flayer->height();
	flayer->move(0, h2);
	flayer->resize(this->width(), flayer->height());
}

void ParupaintWindow::mousePressEvent(QMouseEvent * event)
{
	if(event->buttons() == Qt::RightButton) {
		auto * brush = glass.GetCurrentBrush();
		pool->EndBrushStroke(brush);

		glass.ToggleBrush(old_brush_switch, 1);
		view->SetCurrentBrush(glass.GetCurrentBrush());
		picker->SetColor(glass.GetCurrentBrush()->GetColor());

		old_brush_switch = 0;
		event->accept();
	}
	return QMainWindow::mousePressEvent(event);
}

void ParupaintWindow::keyReleaseEvent(QKeyEvent * event)
{
	if((event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) && !event->isAutoRepeat()){
		OverlayButtonTimer->start(100);
		return;
	}

	return QMainWindow::keyReleaseEvent(event);
}

void ParupaintWindow::keyPressEvent(QKeyEvent * event)
{
	if(event->key() == CanvasKeyChat && !event->isAutoRepeat() && 
	this->hasFocus()){
		chat->setFocus();
		chat->show();
	}
	if(!this->hasFocus()) {

		return QMainWindow::keyPressEvent(event);
	}

	if(!event->isAutoRepeat() && event->key() == CanvasKeyReload &&
			!(event->modifiers() & Qt::NoModifier)){

		if((event->modifiers() & (Qt::ShiftModifier)) &&
		   (event->modifiers() & (Qt::ControlModifier))){

			return client->ReloadCanvas();

		} else if(event->modifiers() & (Qt::ShiftModifier)) {
			return client->ReloadImage();
		}
	}
	if(!event->isAutoRepeat() && event->modifiers() & Qt::ControlModifier){
		if(event->key() == CanvasKeyNew){
			auto * dialog = new ParupaintNewDialog(this);
			dialog->setOriginalDimensions(pool->GetCanvas()->GetWidth(), pool->GetCanvas()->GetHeight());
			dialog->show();
			connect(dialog, &ParupaintNewDialog::NewSignal, [=](int w, int h, bool resize){
				this->New(w, h, resize);
				delete dialog;
			});

		} else if(event->key() == CanvasKeySettings){
			auto * dialog = new ParupaintSettingsDialog(this);
			connect(dialog, &ParupaintSettingsDialog::pixelgridChanged, [=](bool b){
				view->SetPixelGrid(b);
			});
			dialog->show();
		}
	}

	if(event->key() == CanvasKeyNextLayer || 
			event->key() == CanvasKeyPreviousLayer || 
			event->key() == CanvasKeyNextFrame || 
			event->key() == CanvasKeyPreviousFrame){

		int ll = 0, ff = 0;
		if(event->key() == CanvasKeyNextFrame){
			ff ++;
		} else if(event->key() == CanvasKeyPreviousFrame){
			ff --;

		} else if(event->key() == CanvasKeyNextLayer){
			ll++;
		} else if(event->key() == CanvasKeyPreviousLayer){
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
			auto current_layer = pool->GetCanvas()->GetCurrentLayer(),
			     current_frame = pool->GetCanvas()->GetCurrentFrame();

			auto shift = (event->modifiers() & Qt::ShiftModifier),
			     control = (event->modifiers() & Qt::ControlModifier);

			if(ll > 0 && !shift){
				current_layer ++;
			}
			if(ff > 0 && !shift && !control){
				current_frame ++;
			}
			client->SendLayerFrame(current_layer, current_frame, ll, ff, control);
		}

	}
	if(!event->isAutoRepeat() && event->key() == CanvasKeyPreview){

		if(event->modifiers() & Qt::ControlModifier){
			// TODO play the animation
		} else if(event->modifiers() & Qt::ShiftModifier){
			view->SetZoom(1.0);
		} else {
			pool->GetCanvas()->TogglePreview();
		}
		pool->TriggerViewUpdate();
	}

	if(event->key() == BrushKeyPickColor){
		bool global = (event->modifiers() & Qt::ControlModifier);
		QColor col(255,255,255,0);

		auto * brush = glass.GetCurrentBrush();
		const auto pos = brush->GetPosition();
		const auto rect = pool->GetCanvas()->boundingRect();

		if(rect.contains(pos)){
			if(global){
				const auto & img = pool->GetCanvas()->GetCache().toImage();
				col = img.pixel(pos.x(), pos.y());
			} else {
				auto * layer = pool->GetCanvas()->GetLayer(brush->GetLayer());
				if(layer){
					auto * frame = layer->GetFrame(brush->GetFrame());
					if(frame){
						const auto & img = frame->GetImage();
						col = img.pixel(pos.x(), pos.y());
					}
				}
			}
		}

		brush->SetColor(col);
		view->UpdateCurrentBrush(brush);
		picker->SetColor(col);
	}
	// tool
	if(!event->isAutoRepeat() &&
			(event->key() == BrushKeyToolKey1 ||
			 event->key() == BrushKeyToolKey2)){
		int tool = 0;
		if(event->key() == BrushKeyToolKey1) tool = (event->modifiers() & Qt::CTRL) ? 3 : 2;
		if(event->key() == BrushKeyToolKey2) tool = (event->modifiers() & Qt::CTRL) ? 1 : 1;

		auto * brush = glass.GetCurrentBrush();
		if(brush->GetToolType() != 0) tool = 0;

		brush->SetToolType(tool);
		view->UpdateCurrentBrush(brush);
	}
	if(!event->isAutoRepeat() && event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9) {
		int tool = 4;

		if(event->key() >= Qt::Key_1 && event->key() <= Qt::Key_4){
			tool = event->key() - Qt::Key_1;

		} else if(event->key() >= Qt::Key_6 && event->key() <= Qt::Key_9) {
			tool -= (event->key() - Qt::Key_5);
		}
		tool += 2;

		if(glass.GetCurrentBrushNum() != tool){
			if(glass.GetCurrentBrushNum() <= 1){
				old_brush_switch = glass.GetCurrentBrushNum();
			} else {
				// set it to previous, so that it switch to tool
				glass.SetBrush(old_brush_switch);
			}

		}

		glass.ToggleBrush(old_brush_switch, tool);
		view->SetCurrentBrush(glass.GetCurrentBrush());
		picker->SetColor(glass.GetCurrentBrush()->GetColor());
	}
	if(event->key() == Qt::Key_F1 && !event->isAutoRepeat()){
		OverlayState = OVERLAY_STATUS_SHOWN_NORMAL;
		ShowOverlay(true);
	}
	if(event->key() == Qt::Key_Backtab && !event->isAutoRepeat() && !OverlayButtonDown) {
		OverlayState = OVERLAY_STATUS_HIDDEN;
		HideOverlay();

	} else if(OverlayButtonDown){
		OverlayButtonTimer->stop();
	}
	if(event->key() == Qt::Key_Tab){
		OverlayButtonTimer->stop();
		
		if(OverlayState == OVERLAY_STATUS_HIDDEN) {
			OverlayState = OVERLAY_STATUS_SHOWN_SMALL;
			ShowOverlay(false);

		} else if(OverlayState != OVERLAY_STATUS_SHOWN_NORMAL) {
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

void ParupaintWindow::dropEvent(QDropEvent *ev)
{
	if(ev->mimeData()->urls().size() == 1){
		QUrl link = ev->mimeData()->urls().first();
		if(link.isLocalFile()){
			QString file = link.toLocalFile();
			qDebug() << "Loading" << file;
			client->LoadCanvasLocal(file);
		}
	}
}

void ParupaintWindow::dragEnterEvent(QDragEnterEvent *ev)
{
	if(ev->mimeData()->urls().size() != 1) return;
	ev->accept();
}

void ParupaintWindow::UpdateTitle()
{
	setWindowTitle(QString("parupaint"));
}

void ParupaintWindow::New(int w, int h, bool resize)
{
	qDebug() << "New canvas" << w << h;
	client->NewCanvas(w, h, resize);
}

void ParupaintWindow::Connect(QString url)
{
	QSettings cfg;
	client->SetNickname(cfg.value("painter/username").toString());

	client->Connect(url);
}

void ParupaintWindow::Disconnect()
{
	client->Connect("localhost", local_port);
}

void ParupaintWindow::Open(QString filename)
{
	qDebug() << "Open" << filename;

	QSettings cfg;
	cfg.setValue("net/lastopen", filename);

	client->LoadCanvasLocal(filename);
}
QString ParupaintWindow::SaveAs(QString filename)
{
	// TODO handle overwrite here

	if(filename.isEmpty()) filename = ".png";
	if(filename.section(".", 0, 0).isEmpty()) {
		QDateTime time = QDateTime::currentDateTime();
		filename = "drawing_at_"+ time.toString("yyyy-MM-dd_HH.mm.ss") + filename;
	}

	qDebug() << "Saving canvas as" << filename;

	ParupaintPanvasWriter write(pool->GetCanvas());
	write.Save(this->GetSaveDirectory(), filename);

	return filename;
}

QString ParupaintWindow::GetSaveDirectory() const
{
	QSettings cfg;
	QDir saved = cfg.value("client/directory").toString();
	if(!saved.exists()) saved = QDir::current();

	return saved.path();
}

void ParupaintWindow::Command(QString cmd, QString params)
{
	if(params.isEmpty()) return;
	
	qDebug() << cmd << params;
	if(cmd == "load"){
		client->LoadCanvas(params);

	} else if(cmd == "save"){
		client->SaveCanvas(params);

	}
	chat->AddMessage(">> " + cmd + " " + params);
}

void ParupaintWindow::SetLocalHostPort(quint16 port)
{
	local_port = port;
}
