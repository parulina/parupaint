#include "parupaintWindow.h"

#include <QKeyEvent>
#include <QEvent>
#include <QSettings>
#include <QShortcut>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include <QMimeData>
#include <QApplication>
#include <QClipboard>

#include "parupaintVersionCheck.h"
#include "parupaintKeys.h"

#include <algorithm>

#include "core/parupaintPanvasWriter.h"

#include "core/parupaintStrokeStep.h"
#include "core/parupaintBrush.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"
#include "parupaintCanvasObject.h"
#include "parupaintCanvasBrush.h"
#include "parupaintCanvasBanner.h"

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
#include "core/parupaintSnippets.h"
#include "net/parupaintClientInstance.h"

#include <QDebug>

#define PARUPAINTKEY_TO_SHORTCUT(key, lambda) connect(new QShortcut(key_shortcuts->GetKeySequence(key), this), &QShortcut::activated, lambda)

ParupaintWindow::~ParupaintWindow()
{
	delete client;
}

ParupaintWindow::ParupaintWindow() : QMainWindow(), local_port(1108), old_brush_switch(0),
	OverlayButtonDown(false), OverlayState(OVERLAY_STATUS_HIDDEN)
{
	// default keys
	key_shortcuts = new ParupaintKeys(QStringList{

		"sticky_overlay=F1",
		"show_overlay=Tab",
		"hide_overlay=Shift+Tab",

		"dialog_quicksave=Ctrl+K",
		"dialog_open=Ctrl+O",
		"dialog_saveas=Ctrl+L",
		"dialog_new=Ctrl+N",

		"dialog_settings=Ctrl+M",
		"dialog_connect=Ctrl+I",

		"prev_frame=A",
		"next_frame=S",
		"prev_layer=D",
		"next_layer=F",

		"clear_canvas=Ctrl+Backspace",
		"copy=Ctrl+C",
		"paste=Ctrl+V",

		"play_animation=Shift+G",
		"reset_view=Ctrl+G",
		"toggle_preview=G",

		"eraser_switch=E",
		"pick_color=R",
		"pick_global_color=Shift+R",

		"fill_preview=T",

		"toolswitch_fill=W",
		"toolswitch_dotpattern=Ctrl+W",
		"toolswitch_opacity=Shift+W",

		"reload_canvas=Ctrl+Shift+R",
		"reload_image=Ctrl+R",

		"chat=Return"
	});
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
	connect(client, &ParupaintClientInstance::OnDisconnect, this, &ParupaintWindow::OnNetworkDisconnect);
	connect(client, &ParupaintClientInstance::PlaymodeUpdate, [this](ParupaintBrush * brush){
		auto * current_brush = glass.GetCurrentBrush();
		*current_brush = *brush;
		this->view->UpdateCurrentBrush(current_brush);
	});

	canvas_banner = new ParupaintCanvasBanner(this);

	chat =	  new ParupaintChat(this);
	flayer =  new ParupaintFlayer(this);
	picker =  new ParupaintColorPicker(this);
	infobar = new ParupaintInfoBar(this);

	chat->setChatInputPlaceholder(QString("press [%1] to chat.").arg(key_shortcuts->GetKeySequence("chat").toString(QKeySequence::NativeText)).toLower());
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

	fillpreview_timer = new QTimer(this);
	fillpreview_timer->setSingleShot(true);
	connect(fillpreview_timer, &QTimer::timeout, [this](){
		this->pool->GetCanvas()->ClearFillPreview();
		this->pool->GetCanvas()->RedrawCache();
		this->pool->UpdateView();
	});

	/*

	// undo, redo - wat do?
	if(seq == BrushKeyUndo){
		pool->UndoBrushStroke(brush);
	} else if(seq == BrushKeyRedo) {
		pool->RedoBrushStroke(brush);

	*/

	// PARUPAINTKEY_TO_SHORTCUT is a macro that turns keys from key_shortcuts
	// to global shortcuts

	key_shortcuts->Load();

	PARUPAINTKEY_TO_SHORTCUT("dialog_quicksave", [=]{
		QString saved = this->SaveAs(".png");
		QString filelink = QDir(this->GetSaveDirectory()).filePath(saved);
		chat->AddMessage("Quicksaved to '<a href=\""+filelink+"\">"+saved+"</a>'.");
	});

	PARUPAINTKEY_TO_SHORTCUT("dialog_open", [=]{
		QSettings cfg;
		auto lastopen = cfg.value("net/lastopen").toString();
		ParupaintFileDialog * dlg = new ParupaintFileDialog(this, lastopen, "open...");
		dlg->show();
		dlg->activateWindow();

		connect(dlg, &ParupaintFileDialog::EnterSignal, [=](QString str){
			this->Open(str);
			delete dlg;
		});
	});
	PARUPAINTKEY_TO_SHORTCUT("dialog_saveas", [=]{
		QSettings cfg;
		ParupaintFileDialog * dlg = new ParupaintFileDialog(this, ".png", "save as...");
		dlg->show();
		dlg->activateWindow();
		connect(dlg, &ParupaintFileDialog::EnterSignal, [=](QString str){
			QString saved = this->SaveAs(str);
			QString filelink = QDir(this->GetSaveDirectory()).filePath(saved);
			chat->AddMessage("Saved as '<a href=\"file:///"+filelink+"\">"+saved+"</a>'.");
			delete dlg;
		});
	});

	PARUPAINTKEY_TO_SHORTCUT("dialog_new", [=]{
		auto * dialog = new ParupaintNewDialog(this);

		dialog->setOriginalDimensions(pool->GetCanvas()->GetWidth(), pool->GetCanvas()->GetHeight());
		dialog->show();
		dialog->activateWindow();

		connect(dialog, &ParupaintNewDialog::NewSignal, [=](int w, int h, bool resize){
			this->New(w, h, resize);
			delete dialog;
			//TODO remove the delete keyword, connect signal directly to this->new?
		});
	});

	PARUPAINTKEY_TO_SHORTCUT("dialog_settings", [=]{
		auto * dialog = new ParupaintSettingsDialog(this);
		connect(dialog, &ParupaintSettingsDialog::pixelgridChanged, [=](bool b){
			view->SetPixelGrid(b);
		});
		dialog->show();
		dialog->activateWindow();
	});

	PARUPAINTKEY_TO_SHORTCUT("dialog_connect", [=]{
		ParupaintConnectionDialog * dlg = new ParupaintConnectionDialog(this);
		dlg->show();
		dlg->activateWindow();

		connect(dlg, &ParupaintConnectionDialog::ConnectSignal, [=](QString addr){
			// i think i should use QDialog->done(); internally instead of deleting it...
			this->Connect(addr);
			delete dlg;
		});
		connect(dlg, &ParupaintConnectionDialog::DisconnectSignal, [=]{
			this->Disconnect();
			delete dlg;
		});
	});

	key_shortcuts->Save();
	QStringList keylist_html;
	foreach(QString s,key_shortcuts->GetKeys()){
		s.replace("=", ": ").replace("_", " ");
		keylist_html << s;
	}
	std::sort(keylist_html.begin(), keylist_html.end());
	infobar->SetKeyList(keylist_html);

	this->setAcceptDrops(true);

	UpdateTitle();
	
	// restore window pos
	QSettings cfg;
	restoreGeometry(cfg.value("window/maingeometry").toByteArray());
	restoreState(cfg.value("window/mainstate").toByteArray());

	// ctrl + shift + q = quit
	new QShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Q), this, SLOT(close()));

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

// on net disconnect
void ParupaintWindow::OnNetworkDisconnect(QString reason)
{
	if(reason == "SwitchHost") return;
	chat->show();
	chat->AddMessage("You were disconnected from the server.");
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
	if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill){
		//special

		auto x = brush->GetPosition().x(), y = brush->GetPosition().y();
		QRect r = ParupaintFrameBrushOps::stroke(pool->GetCanvas(), x, y, x, y, brush);
		pool->GetCanvas()->RedrawCache(r);

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

	if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill && brush->IsDrawing()) {
		// just for safety
		brush->SetDrawing(false);
	}
	if(brush->IsDrawing() && client->GetDrawMode() == DRAW_MODE_DIRECT){
		auto 	old_x = cbrush->GetPosition().x(),
			old_y = cbrush->GetPosition().y(),
			x = brush->GetPosition().x(),
			y = brush->GetPosition().y();

		QRect r = ParupaintFrameBrushOps::stroke(pool->GetCanvas(), old_x, old_y, x, y, brush);
		pool->GetCanvas()->RedrawCache(r);

	}
	cbrush->SetPosition(brush->GetPosition());
	client->SendBrushUpdate(brush);

	// only normal brush for strokes plz
	if(brush->GetToolType() != ParupaintBrushToolTypes::BrushToolNone) return;
	if(client->GetDrawMode() != DRAW_MODE_DIRECT){
		auto *stroke = brush->GetCurrentStroke();
		if(stroke != nullptr){
			stroke->AddStroke(new ParupaintStrokeStep(*brush));
		}
	}
}

void ParupaintWindow::PenDrawStop(ParupaintBrush* brush){

	if(brush->GetToolType() == ParupaintBrushToolTypes::BrushToolFloodFill) return;

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
	view->UpdateCurrentBrush(brush);
	flayer->SetMarkedLayerFrame(l, f);
	infobar->SetCurrentLayerFrame(l+1, f+1);
}

void ParupaintWindow::SelectFrame(int l, int f)
{
	auto brush = glass.GetCurrentBrush();
	brush->SetLayer(l);
	brush->SetFrame(f);
	view->UpdateCurrentBrush(brush);
	client->SendBrushUpdate(brush);
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
	if(!chat->hasFocus()) chat->hide();
	if(!flayer->hasFocus()) flayer->hide();
	if(!picker->hasFocus()) picker->hide();

	UpdateOverlay();
}
void ParupaintWindow::UpdateOverlay()
{
	canvas_banner->move(this->width()/2 - canvas_banner->width()/2, 0);

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
	if(this->hasFocus()){
		if(event->buttons() == Qt::RightButton) {
			auto * brush = glass.GetCurrentBrush();
			pool->EndBrushStroke(brush);

			glass.ToggleBrush(old_brush_switch, 1);
			view->SetCurrentBrush(glass.GetCurrentBrush());
			picker->SetColor(glass.GetCurrentBrush()->GetColor());

			old_brush_switch = 0;
			event->accept();
		}
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
	QString shortcut_name = key_shortcuts->Match(event->key(), event->modifiers());
	if(!event->isAutoRepeat() && event->key()){
		// single click keys
		if(shortcut_name == "chat"){
			chat->setFocus();
			chat->show();
		}
	}
	if(!event->isAutoRepeat() && event->key() == Qt::Key_Escape){
		if(view->HasPastePreview()){
			view->UnsetPastePreview();
		}
	}

	if(!event->isAutoRepeat()){
		// ui stuff
		if(shortcut_name == "sticky_overlay"){
			OverlayState = OVERLAY_STATUS_SHOWN_NORMAL;
			ShowOverlay(true);
			//
		// canvas stuff
		} else if(shortcut_name == "eraser_switch") {
			auto brush = glass.GetCurrentBrush();

			// safety
			pool->EndBrushStroke(brush);

			glass.ToggleBrush(old_brush_switch, 1);
			view->SetCurrentBrush(glass.GetCurrentBrush());
			picker->SetColor(glass.GetCurrentBrush()->GetColor());

			old_brush_switch = 0;

		} else if(shortcut_name == "reload_canvas"){
			canvas_banner->Show(2000, "reloaded canvas");
			client->ReloadCanvas();

		} else if(shortcut_name == "reload_image"){
			canvas_banner->Show(2000, "reloaded image");
			client->ReloadImage();

		} else if(shortcut_name == "play_animation"){
			// TODO
			pool->TriggerViewUpdate();

		} else if(shortcut_name == "reset_view"){
			view->SetZoom(1.0);
			pool->TriggerViewUpdate();

		} else if(shortcut_name == "toggle_preview"){
			pool->GetCanvas()->TogglePreview();
			canvas_banner->Show(2000, 
					QString("preview ") + (pool->GetCanvas()->IsPreview() ? "on" : "off"));
			pool->TriggerViewUpdate();

		} else if(shortcut_name == "copy"){
			if(view->hasFocus()){
				if(view->HasPastePreview()) view->UnsetPastePreview();
				QImage img = pool->GetCanvas()->GetCurrentLayerFrameImage();
				QClipboard * clip = QApplication::clipboard();
				if(clip){
					clip->setImage(img);
				}
			}

		} else if(shortcut_name == "paste"){
			if(view->hasFocus()){
				QClipboard * clip = QApplication::clipboard();
				if(clip){
					QImage img = clip->image();
					if(!img.isNull()){
						if(view->HasPastePreview()){
							view->UnsetPastePreview();
							if(client){
								auto brush = glass.GetCurrentBrush();
								int 	l = pool->GetCanvas()->GetCurrentLayer(),
									f = pool->GetCanvas()->GetCurrentFrame();
								double	x = brush->GetPosition().x(),
									y = brush->GetPosition().y();

								client->PasteLayerFrameImage(l, f, x, y, img);
							}
							// skip view update for now
						} else {
							view->SetPastePreview(img);
						}
					}
				}
			}

		} else if(shortcut_name == "clear_canvas"){
			qDebug() << "Clear canvas";
			auto * brush = glass.GetCurrentBrush();
			int l = brush->GetLayer(),
			    f = brush->GetFrame();
			client->FillCanvas(l, f, brush->GetColorString());

		} else if(shortcut_name == "fill_preview"){

			ParupaintBrush * brush = glass.GetCurrentBrush();
			QImage fimg = pool->GetCanvas()->GetCurrentLayerFrameImage();

			ParupaintFillHelper help(fimg);
			help.fill(brush->GetPosition().x(), brush->GetPosition().y(), brush->GetColor().rgba());
			QImage mask = help.mask();

			this->pool->GetCanvas()->SetFillPreview(mask);
			this->pool->GetCanvas()->RedrawCache();
			this->pool->UpdateView();

			if(fillpreview_timer->isActive()) fillpreview_timer->stop();
			fillpreview_timer->start(500);

		} else if(shortcut_name == ""){

		} else if(shortcut_name.startsWith("toolswitch_")){

			int tool = 0;
			if(shortcut_name.endsWith("fill"))
				tool = ParupaintBrushToolTypes::BrushToolFloodFill;

			if(shortcut_name.endsWith("dotpattern"))
				tool = ParupaintBrushToolTypes::BrushToolDotPattern;

			if(shortcut_name.endsWith("opacity"))
				tool = ParupaintBrushToolTypes::BrushToolOpacityDrawing;

			auto * brush = glass.GetCurrentBrush();
			if(brush->GetToolType() != ParupaintBrushToolTypes::BrushToolNone)
				tool = ParupaintBrushToolTypes::BrushToolNone;

			brush->SetToolType(tool);
			view->UpdateCurrentBrush(brush);

		}
	}
	// repeating hotkeys
	if(true) {
		// can repeat
		if(shortcut_name.endsWith("_frame") ||
		shortcut_name.endsWith("_layer")) {

			int ll = 0, ff = 0;

			if(shortcut_name == "next_frame"){
				ff ++;
			} else if(shortcut_name == "prev_frame"){
				ff --;

			} else if(shortcut_name == "next_layer"){
				ll++;
			} else if(shortcut_name == "prev_layer"){
				ll--;
			}

			if(!OverlayButtonDown) {
				// Do a local check for boundaries
				pool->GetCanvas()->AddLayerFrame(true, ll, ff);

				auto brush = glass.GetCurrentBrush();
				brush->SetLayer(pool->GetCanvas()->GetCurrentLayer());
				brush->SetFrame(pool->GetCanvas()->GetCurrentFrame());
				client->SendBrushUpdate(brush);
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
		else if(shortcut_name == "pick_color" || shortcut_name == "pick_global_color"){
			bool global = (shortcut_name == "pick_global_color");
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
		//TODO notify user of custom brush switch...
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
	cfg.setValue("window/maingeometry", saveGeometry());
	cfg.setValue("window/mainstate", saveState());
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
	qDebug() << cmd << params;
	if(!params.isEmpty()){
		if(cmd == "load"){
			client->LoadCanvas(params);

		} else if(cmd == "save"){
			client->SaveCanvas(params);

		// record commands
		} else if(cmd == "play") {
			client->PlayRecord(params, false);

		} else if(cmd == "script") {
			client->PlayRecord(params, true);
		}
	}

	if(cmd == "key"){
		if(params.isEmpty()){
			auto list = key_shortcuts->GetKeys();
			list.sort();
			QString str = list.join("<br/>");
			str.append("<br/>List of keys, page up and down to scroll.<br/>Dialog hotkeys requires restart.<br/>Usage: /key name=shortcut");
			return chat->AddMessage(str);
		}
		key_shortcuts->AddKey(params);
		key_shortcuts->Save();
	}
	chat->AddMessage(">> " + cmd + " " + params);
}

void ParupaintWindow::SetLocalHostPort(quint16 port)
{
	local_port = port;
}
