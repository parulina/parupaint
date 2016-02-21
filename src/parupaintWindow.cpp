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
#include <QVBoxLayout>
#include <QMessageBox>

#include "parupaintKeys.h"

#include <algorithm>

#include "core/parupaintPanvasInputOutput.h"

#include "core/parupaintBrush.h"
#include "core/parupaintBrushGlass.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasScene.h"
#include "parupaintVisualCanvas.h"
#include "parupaintVisualCursor.h"

#include "core/parupaintLayer.h"
#include "core/parupaintFrame.h"

#include "overlay/parupaintChat.h"
#include "overlay/parupaintFlayer.h"
#include "overlay/parupaintColorPicker.h"
#include "overlay/parupaintNetJoinPrompt.h"
#include "overlay/parupaintNetPlayerList.h"
#include "overlay/parupaintInfoBar.h"


#include "core/parupaintFrameBrushOps.h"
#include "core/parupaintSnippets.h"
#include "net/parupaintClientInstance.h"

#include <QDebug>

#define PARUPAINTKEY_TO_SHORTCUT(key, w, f) \
	connect(new QShortcut(key_shortcuts->keySequence(key), this), &QShortcut::activated, w, f)

ParupaintWindow::ParupaintWindow(QWidget * parent) : QMainWindow(parent),
	overlay_state(overlayHiddenState), canvas_state(noCanvasState),
	overlay_button(false),
	tablet_pen_switch(false)
{
	// default keys
	key_shortcuts = new ParupaintKeys(QStringList{

		"overlay_showfull=F1",
		// dunno what to do with this...
		//"overlay_show=Tab",
		//"overlay_hide=Shift+Tab",

		"action_quicksave=Ctrl+K",
		"action_open=Ctrl+O",
		"action_saveas=Ctrl+L",
		"action_new=Ctrl+N",
		"action_settings=Ctrl+M",
		"action_connect=Ctrl+I",

		"canvas_fliph=H",
		"canvas_flipv=Shift+H",
		"canvas_clear=Ctrl+Backspace",
		"canvas_reset=Ctrl+G",
		"canvas_play=Shift+G",
		"canvas_preview=G",

		"prev_frame=A",
		"next_frame=S",
		"prev_layer=D",
		"next_layer=F",

		"frame_opacityinc=P",
		"frame_opacitydec=O",

		"copy=Ctrl+C",
		"paste=Ctrl+V",


		"brush_eraser=E",
		"brush_pencil=B",
		"brush_fillpreview=T",

		"pick_layer_color=R",
		"pick_canvas_color=Shift+R",

		"tool_fill=Q",
		"tool_opacity=Shift+Q",
		"tool_dotpattern=W",
		"tool_line=Shift+W",

		"reload_canvas=Ctrl+Shift+R",
		"reload_image=Ctrl+R",

		"chat=Return",
		"exit=Ctrl+Shift+Q"
	});
	// create brushglass
	brushes = new ParupaintBrushGlass(this);
	brushes->loadBrushes();
	brushes->saveBrushes();

	// create view and scene
	view = new ParupaintCanvasView(this);
	view->setScene((scene = new ParupaintCanvasScene(view)));

	// parupaintWindow_pen.cpp
	connect(view, &ParupaintCanvasView::pointerPress, this, &ParupaintWindow::OnPenPress);
	connect(view, &ParupaintCanvasView::pointerMove, this, &ParupaintWindow::OnPenMove);
	connect(view, &ParupaintCanvasView::pointerRelease, this, &ParupaintWindow::OnPenRelease);
	connect(view, &ParupaintCanvasView::pointerPointer, this, &ParupaintWindow::OnPenPointer);
	connect(view, &ParupaintCanvasView::pointerScroll, this, &ParupaintWindow::OnPenScroll);

	// update the overlay items with the viewport change
	connect(view, &ParupaintCanvasView::viewportChange, this, &ParupaintWindow::updateOverlay);

	// create the client and connect it
	client = new ParupaintClientInstance(scene, this);
	connect(client, &ParupaintClientInstance::onChatMessage, this, &ParupaintWindow::addChatMessage);
	connect(client, &ParupaintClientInstance::onDisconnect, this, &ParupaintWindow::OnNetworkDisconnect);
	connect(client, &ParupaintClientInstance::onConnect, this, &ParupaintWindow::OnNetworkConnect);


	chat = 		new ParupaintChat(this);
	picker = 	new ParupaintColorPicker(this);
	flayer = 	new ParupaintFlayer(this);
	infobar =	new ParupaintInfoBar(this);
	netjoin =	new ParupaintNetJoinPrompt(this);
	netlist = 	new ParupaintNetPlayerList(this);

	connect(client, &ParupaintClientInstance::onConnect, netjoin, &QWidget::show);
	connect(client, &ParupaintClientInstance::onDisconnect, netjoin, &QWidget::hide);
	connect(client, &ParupaintClientInstance::onSpectateChange, netjoin, &QWidget::setVisible);

	connect(netjoin, &ParupaintNetJoinPrompt::wantJoin, this, &ParupaintWindow::doJoin);

	// chatactivity -> chatbubble
	chat->setChatInputPlaceholder(
		QString("press [%1] to chat.").arg(key_shortcuts->keyString("chat")).toLower()
	);
	connect(chat, &ParupaintChat::onActivity, [this](){
		this->client->doChat();
	});
	
	connect(flayer, &ParupaintFlayer::onHighlightChange, scene->canvas(), &ParupaintVisualCanvas::current_lf_update);
	connect(flayer, &ParupaintFlayer::onHighlightChange, infobar->status(), &ParupaintInfoBarStatus::setLayerFrame);
	connect(flayer, &ParupaintFlayer::onHighlightChange, [&](int l, int f){
		ParupaintBrush * brush = this->brushes->brush();
		brush->setLayerFrame(l, f);
		this->client->doBrushUpdate(brush);
	});

	connect(scene->canvas(), &ParupaintVisualCanvas::onCurrentLayerFrameChange, flayer, &ParupaintFlayer::current_lf_update);
	connect(scene->canvas(), &ParupaintVisualCanvas::onCurrentLayerFrameChange, infobar->status(), &ParupaintInfoBarStatus::setLayerFrame);
	connect(scene->canvas(), &ParupaintVisualCanvas::onCurrentLayerFrameChange, [&](int l, int f){
		ParupaintBrush * brush = this->brushes->brush();
		brush->setLayerFrame(l, f);
		this->client->doBrushUpdate(brush);
	});

	// canvas content -> flayer content
	connect(scene->canvas(), &ParupaintVisualCanvas::onCanvasContentChange, flayer, &ParupaintFlayer::canvas_content_update);
	connect(scene->canvas(), &ParupaintVisualCanvas::onCanvasResize, infobar->status(), &ParupaintInfoBarStatus::setDimensions);

	// chat message -> chat message
	connect(chat, &ParupaintChat::Message, this, &ParupaintWindow::doChat);

	// color picker -> brushglass, visual brush
	connect(picker, &ParupaintColorPicker::ColorChange, brushes, &ParupaintBrushGlass::color_change);
	connect(picker, &ParupaintColorPicker::ColorChange, scene->mainCursor(), &ParupaintVisualCursor::setColor);

	// visual brush -> color picker
	connect(scene->mainCursor(), &ParupaintBrush::onColorChange, picker, &ParupaintColorPicker::color_change);


	connect(infobar, &ParupaintInfoBar::onStatusClick, [&](const QUrl & url){
		const QString str = url.toString();
		qDebug() << str;

		if(str == "#f1_notice"){
			this->showOverlay(overlayExpandedState);
		} else if(str == "#connected"){
			this->showConnectDialog();
		} else if(str == "#dimensions"){
			this->showNewDialog();
		}
	});

	// set a nice layout to the window, infobar and view
	QWidget * central_widget = new QWidget(this);
		QVBoxLayout * main_layout = new QVBoxLayout;
			main_layout->addWidget(infobar, 0, Qt::AlignBottom);
			main_layout->addWidget(view, 1);
			main_layout->addWidget(flayer, 0, Qt::AlignBottom);
		central_widget->setLayout(main_layout);
	this->setCentralWidget(central_widget);


	// set up some overlay keybinds
	overlay_timeout = new QTimer(this);
	overlay_timeout->setSingleShot(true);
	connect(overlay_timeout, &QTimer::timeout, this, &ParupaintWindow::hideOverlay);

	overlay_button_timeout = new QTimer(this);
	overlay_button_timeout->setSingleShot(true);
	connect(overlay_button_timeout, &QTimer::timeout, [&](){
		this->overlay_button = false;
	});

	// load keys and connect them
	// TODO QAction? right now it's using QShortcut
	key_shortcuts->loadKeys();
	PARUPAINTKEY_TO_SHORTCUT("action_quicksave", 	this, &ParupaintWindow::doQuickSave);
	PARUPAINTKEY_TO_SHORTCUT("action_open", 	this, &ParupaintWindow::showOpenDialog);
	PARUPAINTKEY_TO_SHORTCUT("action_saveas", 	this, &ParupaintWindow::showSaveAsDialog);
	PARUPAINTKEY_TO_SHORTCUT("action_new", 		this, &ParupaintWindow::showNewDialog);
	PARUPAINTKEY_TO_SHORTCUT("action_settings", 	this, &ParupaintWindow::showSettingsDialog);
	PARUPAINTKEY_TO_SHORTCUT("action_connect", 	this, &ParupaintWindow::showConnectDialog);
	PARUPAINTKEY_TO_SHORTCUT("exit", 		this, &ParupaintWindow::close);

	key_shortcuts->saveKeys();

	// restore window pos
	QSettings cfg;
	if(cfg.value("window/restore", true).toBool()){
		this->restoreGeometry(cfg.value("window/maingeometry").toByteArray());
		this->restoreState(cfg.value("window/mainstate").toByteArray());
	}

	this->hideOverlay();
	scene->canvas()->redraw();

	// accept drops...
	this->setMinimumSize(QSize(700, 700));
	this->setAcceptDrops(true);

	// do an update of the initial brush
	scene->updateMainCursor(brushes->brush());

	this->setFocusProxy(view);
	this->setFocusPolicy(Qt::StrongFocus);

	// make sure the overlay controls are able to be above this
	this->centralWidget()->lower();
	this->show();
}

ParupaintClientInstance * ParupaintWindow::networkClient()
{
	return client;
}

void ParupaintWindow::OnNetworkConnect()
{
	QUrl url = client->url();
	infobar->status()->setConnectedTo(url.host());

	if(url.host() == "localhost") return;
	chat->AddMessage("You are connected to " + url.host());
}
void ParupaintWindow::OnNetworkDisconnect(QString reason)
{
	chat->show();
	chat->AddMessage("You were disconnected from the server.");
	infobar->status()->setConnectedTo("");

	QApplication::alert(this, 2000);
}

void ParupaintWindow::setCanvasState(canvasStates state)
{
	if(canvas_state == state) return;

	if(state == canvasDrawingState)            view->setCursor(Qt::BlankCursor);
	else if(state == canvasMovingState)        view->setCursor(Qt::ClosedHandCursor);
	else if(state == canvasZoomingState)       view->setCursor(Qt::SizeVerCursor);
	else {
		view->setCursor(Qt::BlankCursor);
	}
	canvas_state = state;
}


void ParupaintWindow::showOverlay(overlayStates state)
{
	overlay_state = state;

	overlay_timeout->stop();
	if(overlay_state == overlayNormalState) {
		QSettings cfg;
		overlay_timeout->start(cfg.value("client/tabtimeout", 1400).toInt());
	}

	chat->show();
	flayer->show();
	picker->show();

	this->updateOverlay();
}

inline bool widgetContainsCursor(QWidget * widget)
{
	if(!widget) return false;
	
	return (widget->rect().contains(widget->mapFromGlobal(QCursor::pos())));
}

void ParupaintWindow::hideOverlay()
{
	if(overlay_state != overlayHiddenState) {

		if(widgetContainsCursor(chat) ||
		   widgetContainsCursor(picker) ||
		   widgetContainsCursor(flayer)) {
			return;
		}
	}


	if(!chat->hasFocus())
		chat->hide();
	picker->hide();
	flayer->hide();

	overlay_state = overlayHiddenState;
	this->updateOverlay();
}
void ParupaintWindow::updateOverlay()
{
	if(view_pos.isNull()) view_pos = view->pos();
	if(view->pos() != view_pos){

		QPointF diff = view->pos() - view_pos;
		view->moveView(diff);
		view_pos = view->pos();
	}

	infobar->setFixedHeight(overlay_state == overlayExpandedState ? 180 : 30);
	const QRect inner_size = QRect(view->pos(), view->viewport()->size()).adjusted(1, 1, 1, 1);

	picker->move(inner_size.topLeft());
	chat->move(inner_size.topRight() - QPoint(chat->width(), 0));

	netjoin->move(QPoint(inner_size.left(), inner_size.bottom() - netjoin->height()));
	netlist->move(inner_size.topLeft() + QPoint(0, picker->height()));
}

void ParupaintWindow::keyReleaseEvent(QKeyEvent * event)
{
	if(event->key() == Qt::Key_Space){
		origin_pen = current_pen; // fix that weird rant-bug
		this->setCanvasState(noCanvasState);
	}
	if((event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) && !event->isAutoRepeat()){

		overlay_button_timeout->start(100);
		event->accept();
	}
	return QMainWindow::keyReleaseEvent(event);
}

void ParupaintWindow::keyPressEvent(QKeyEvent * event)
{
	QString shortcut_name = key_shortcuts->keyName(event->key(), event->modifiers());
	if(!event->isAutoRepeat() && event->key()){
		// single click keys
		if(shortcut_name == "chat"){
			chat->setFocus();
			chat->show();
		}
	}
	// Escape to cancel
	if(!event->isAutoRepeat() && event->key() == Qt::Key_Escape){

		ParupaintBrush * brush = brushes->brush();
		if(scene->canvas()->hasPastePreview()){
			scene->canvas()->setPastePreview();
		} else if(brush){
			if(brush->tool() != ParupaintBrushToolTypes::BrushToolNone){

				brush->setTool(ParupaintBrushToolTypes::BrushToolNone);
				scene->updateMainCursor(brush);
				client->doBrushUpdate(brush);
			}
		}
	}
	if(event->key() == Qt::Key_Space){
		if(canvas_state == noCanvasState){
			if(event->modifiers() & Qt::ShiftModifier){
				this->setCanvasState(canvasBrushZoomingState);

			} else if(event->modifiers() & Qt::ControlModifier){
				this->setCanvasState(canvasZoomingState);

			} else {
				this->setCanvasState(canvasMovingState);
			}
			origin_pen = current_pen;
			origin_zoom = view->zoom();
			return;
		}
	}

	if(!event->isAutoRepeat()){
		// ui stuff
		if(shortcut_name.startsWith("overlay_")){
			if(shortcut_name.endsWith("showfull")){
				if(overlay_state == overlayHiddenState){
					this->showOverlay(overlayExpandedState);
				} else {
					this->hideOverlay();
				}
			}
		// canvas stuff
		} else if(shortcut_name == "reload_canvas"){
			view->showToast("reloaded canvas", 2000);
			client->doReloadCanvas();

		} else if(shortcut_name == "reload_image"){
			view->showToast("reloaded image", 2000);
			client->doReloadImage();

		} else if(shortcut_name == "copy"){
			if(view->hasFocus()){
				if(scene->canvas()->hasPastePreview())
					scene->canvas()->setPastePreview();
				QImage img = scene->canvas()->currentCanvasFrame()->image();
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
						QPointF p = brushes->brush()->position();

						if(scene->canvas()->hasPastePreview()){
							scene->canvas()->setPastePreview();
							if(client){
								int 	l = scene->canvas()->currentLayer(),
									f = scene->canvas()->currentFrame();

								client->doPasteImage(l, f, p.x(), p.y(), img);
							}
							// skip view update for now
						} else {
							scene->canvas()->setPastePreview(img, p);
						}
					}
				}
			}

		} else if(shortcut_name.startsWith("canvas_")){
			qDebug() << "canvas key";

			if(shortcut_name.endsWith("fliph")) {
				view->flipView(true, false);

			} else if(shortcut_name.endsWith("flipv")) {
				view->flipView(false, true);

			} else if(shortcut_name.endsWith("clear")){
				ParupaintBrush * brush = brushes->brush();
				client->doFill(brush->layer(), brush->frame(), brush->colorString());

			} else if(shortcut_name.endsWith("reset")){
				if(view->zoom() != 1.0){
					view->setZoom(1.0);
					view->showToast("100 %");
				} else {
					view->resetFlip();
				}
			} else if(shortcut_name.endsWith("play")){
				view->showToast("can't play animations yet - sorry!", 1000);

			} else if(shortcut_name.endsWith("preview")){
				scene->canvas()->setPreview(!scene->canvas()->isPreview());
				QString str = QString("preview %1").arg(scene->canvas()->isPreview() ? "on" : "off");
				view->showToast(str, 2000);
			}

		} else if(shortcut_name.startsWith("brush_")){
			if(shortcut_name.endsWith("pencil")){
				brushes->clearToggle();
				brushes->setBrush(0);
				scene->updateMainCursor(brushes->brush());
				view->showToast(brushes->brush()->name(), 600);

			} else if(shortcut_name.endsWith("eraser")){
				brushes->toggleBrush(1);
				scene->updateMainCursor(brushes->brush());

			} else if(shortcut_name.endsWith("fillpreview")){
				ParupaintBrush * brush = brushes->brush();
				QImage fimg = scene->canvas()->currentCanvasFrame()->image();

				ParupaintFillHelper help(fimg);
				help.fill(brush->x(), brush->y(), brush->rgba());
				QImage mask = help.mask();

				this->scene->canvas()->setFillPreview(mask);
			}
		} else if(shortcut_name.startsWith("tool_")){

			ParupaintBrush * brush = brushes->brush();
			int tool = ParupaintBrushToolTypes::BrushToolNone;

			if(shortcut_name.endsWith("fill")) {
				if(brush->tool() == ParupaintBrushToolTypes::BrushToolNone)
					tool = ParupaintBrushToolTypes::BrushToolFloodFill;
			}
			if(shortcut_name.endsWith("dotpattern")){
				switch(brush->tool()){

					case ParupaintBrushToolTypes::BrushToolNone:
						tool = ParupaintBrushToolTypes::BrushToolDotShadingPattern; break;
					case ParupaintBrushToolTypes::BrushToolDotShadingPattern:
						tool = ParupaintBrushToolTypes::BrushToolDotHighlightPattern; break;
					case ParupaintBrushToolTypes::BrushToolDotHighlightPattern:
						tool = ParupaintBrushToolTypes::BrushToolCrossPattern; break;
					case ParupaintBrushToolTypes::BrushToolCrossPattern:
						tool = ParupaintBrushToolTypes::BrushToolGrid; break;
					case ParupaintBrushToolTypes::BrushToolGrid:
						tool = ParupaintBrushToolTypes::BrushToolNone; break;
				}
			}
			if(shortcut_name.endsWith("line")){
				if(brush->tool() == ParupaintBrushToolTypes::BrushToolNone)
					tool = ParupaintBrushToolTypes::BrushToolLine;
			}
			if(shortcut_name.endsWith("opacity")){
				if(brush->tool() == ParupaintBrushToolTypes::BrushToolNone)
					tool = ParupaintBrushToolTypes::BrushToolOpacityDrawing;
			}
			brush->setTool(tool);
			scene->updateMainCursor(brush);
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

			if(!overlay_button) {
				// Do a local check for boundaries
				scene->canvas()->addCurrentLayerFrame(ll, ff);

				ParupaintBrush * brush = brushes->brush();
				brush->setLayerFrame(
					scene->canvas()->currentLayer(),
					scene->canvas()->currentFrame());
				client->doBrushUpdate(brush);

			} else {
				int cl = scene->canvas()->currentLayer(),
				     cf = scene->canvas()->currentFrame();

				auto shift = (event->modifiers() & Qt::ShiftModifier),
				     control = (event->modifiers() & Qt::ControlModifier);

				if(ll > 0 && !shift) 		 cl ++;
				if(ff > 0 && !shift && !control) cf ++;
				client->doLayerFrameChange(cl, cf, ll, ff, control);
			}
		}
		else if(shortcut_name == "pick_layer_color" || shortcut_name == "pick_canvas_color"){
			bool global = (shortcut_name == "pick_canvas_color");
			QColor col(255,255,255,0);

			ParupaintBrush * brush = brushes->brush();
			const QPoint pos = brush->pixelPosition();
			const QRectF rect = scene->canvas()->boundingRect().toRect();

			if(rect.contains(pos)){
				if(global){
					const QImage & img = scene->canvas()->canvasCache().toImage();
					col = img.pixel(pos.x(), pos.y());
				} else {
					ParupaintLayer * layer = scene->canvas()->layerAt(brush->layer());
					if(layer){
						ParupaintFrame * frame = layer->frameAt(brush->frame());
						if(frame){
							const QImage & img = frame->image();
							col = QColor::fromRgba(img.pixel(pos.x(), pos.y()));
						}
					}
				}
			}
			picker->show();
			brush->setColor(col);
			scene->updateMainCursor(brush);

		} else if(shortcut_name.endsWith("_opacityinc") || shortcut_name.endsWith("_opacitydec")){
			ParupaintFrame * frame = scene->canvas()->currentCanvasFrame();
			if(frame){
				qreal op = frame->opacity() + (shortcut_name.endsWith("_opacityinc") ? 0.1 : -0.1);
				client->doLayerFrameAttribute(scene->canvas()->currentLayer(), scene->canvas()->currentFrame(), "frame-opacity", op);
			}
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

		ParupaintBrush * brush = brushes->setBrush(tool);
		if(brush){
			brushes->clearToggle();

			scene->updateMainCursor(brush);

			QString toaststr = brush->name();
			if(brushes->brushNum() == tool)
				toaststr = QString("custom brush %1: %2").arg(tool).arg(brush->name());
			view->showToast(toaststr, 600);
		}
	}
	if(event->key() == Qt::Key_Backtab && !event->isAutoRepeat()){
		// if(overlay_button) ->stop
		if(overlay_button)   overlay_button_timeout->stop();
		if(!overlay_button) {
			overlay_state = overlayHiddenState;
			this->hideOverlay();
		}
	}
	if(event->key() == Qt::Key_Tab){
		overlay_button_timeout->stop();
		overlay_button = true;
		
		if(overlay_state == overlayHiddenState)
			this->showOverlay(overlayNormalState);
		else if(overlay_state == overlayNormalState)
			this->showOverlay(overlayExpandedState);

		return;
	}
	if(event->key() == Qt::Key_Space && !event->isAutoRepeat()){
		if(overlay_button){
			// this used to be clearing strokes
			// i don't know what to do with this combo now...
		}
	}
	return QMainWindow::keyPressEvent(event);
}

void ParupaintWindow::closeEvent(QCloseEvent *)
{
	QSettings cfg;
	if(cfg.value("window/restore", true).toBool()){
		cfg.setValue("window/maingeometry", saveGeometry());
		cfg.setValue("window/mainstate", saveState());
	}

	brushes->saveBrushes();
}


void ParupaintWindow::resizeEvent(QResizeEvent* event)
{
	this->updateOverlay();
	QMainWindow::resizeEvent(event);
}

void ParupaintWindow::dropEvent(QDropEvent *ev)
{
	if(ev->mimeData()->urls().size() == 1){
		QUrl link = ev->mimeData()->urls().first();
		if(link.isLocalFile()){
			QFileInfo file(link.toLocalFile());
			if(file.size() > (1000 * 1000 * 20)){
				chat->AddMessage("File is too big.");
				return;
			}

			QMessageBox box(this);
			box.setIcon(QMessageBox::NoIcon);
			box.setWindowTitle("Confirm load");
			box.setText(QString("Are you sure you want to load '%1'?").arg(file.fileName()));
			box.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			if(box.exec() == QMessageBox::Yes){
				client->doLoadLocal(file.filePath());
			}
		}
	}
}

void ParupaintWindow::dragEnterEvent(QDragEnterEvent *ev)
{
	if(ev->mimeData()->urls().size() != 1) return;
	ev->accept();
}

void ParupaintWindow::doConnect(const QString & url)
{
	if(url.isEmpty()) return;

	client->Connect(url);
}

void ParupaintWindow::doDisconnect()
{
	client->Connect(local_server);
}

void ParupaintWindow::doJoin()
{
	if(!client->connected()) return;
	if(!client->remoteHasPassword()){
		return client->doJoin();
	}
	this->showPasswordDialog();
}

void ParupaintWindow::doJoinPassword(const QString & password)
{
	if(!client->connected()) return;
	client->doJoin(password);
}

void ParupaintWindow::doQuickSave()
{
	QString filepath = this->saveDir().path() + "/.png";
	qDebug() << "doQuickSave" << filepath;

	QFileInfo fi(this->doSaveAs(filepath));
	this->addChatMessage("Quicksaved to '<a href=\""+fi.absoluteFilePath()+"\">"+fi.fileName()+"</a>'.");
}

void ParupaintWindow::doOpen(const QString & file)
{
	if(file.isEmpty()) return;

	client->doLoadLocal(file);
}
QString ParupaintWindow::doSaveAs(const QString & file)
{
	QString err, filename(file.isEmpty() ? (this->saveDir().path() + "/.png") : file);
	bool ret;
	if(!(ret = ParupaintPanvasInputOutput::savePanvas(scene->canvas(), filename, err))){
		this->addChatMessage(err);
	}

	return filename;
}
void ParupaintWindow::doNew(int w, int h, bool resize)
{
	client->doNew(w, h, resize);
}

void ParupaintWindow::doChat(const QString & msg)
{
	if(msg.startsWith('/')){
		QString cmd = msg;
		QString params = "";
		if(msg.indexOf(" ") != -1){
			cmd = msg.section(" ", 0, 0);
			params = msg.section(" ", 1);
		}
		cmd = cmd.mid(1);
		return this->doCommand(cmd, params);
	}
	client->doChat(msg);
}
void ParupaintWindow::doCommand(const QString & cmd, const QString & params)
{
	this->addChatMessage(cmd + " " + params);
	if(cmd == "join"){
		return client->doJoin(params);
	}
	if(cmd == "leave"){
		return client->doLeave();
	}

	if(cmd == "key"){
		if(params.isEmpty()){
			QStringList list = key_shortcuts->keyListString();
			list.sort();
			list += "<br/>Usage: /key name=shortcut";
			list += "Set keys. Dialog hotkeys requires restart.";

			return chat->AddMessage(list.join("<br/>"));
		}
		key_shortcuts->setKey(params);
		key_shortcuts->saveKeys();
	}
	else if(cmd == "load"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /load file<br/>Load a file on the server.");
		client->doLoad(params);
	}
	else if(cmd == "save"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /save file<br/>Saves to a file on the server.");
		client->doSave(params);
	}
	/*
	else if(cmd == "play"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /play file<br/>Plays a record on the server.");
		client->PlayRecord(params, false);
	}
	else if(cmd == "script"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /script file<br/>Plays a script (might be laggy!).");
		client->PlayRecord(params, true);
	}
	*/
	else if(cmd == "connect"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /connect hostname [port]<br/>Reconnect to a different server.");

		// FIXME use default port
		QString host = params.section(" ", 0, 0);
		int port = 0;
		if(params.contains(' ')) port = params.section(" ", 1).toInt();

		this->doConnect(host + (port > 0 ? QString(" %1").arg(port) : ""));
	}
	else if(cmd == "disconnect"){
		this->doDisconnect();
		if(params.isEmpty()){
			this->doConnect("localhost");
		}
	}
	else if(cmd == "name"){
		if(params.isEmpty()) return chat->AddMessage("Usage: /name name <br/>Set your nickname.");
		if(params.length() > 24) return;
		this->doUserName(params);
	}
	else if(cmd == "clear"){
		chat->ClearMessages();
	}
	else if(cmd == "help"){
		QStringList list = {
			"Server commands: /load /save /play /script",
			"Client commands: /connect /disconnect /name /clear"
		};
		return chat->AddMessage(list.join("<br/>"));
	}
}

void ParupaintWindow::doSessionPassword(const QString & sessionpw)
{
	if(sessionpw.length() > 64) return;

	client->doInfo("sessionpassword", sessionpw);
}

void ParupaintWindow::doUserName(const QString & username)
{
	if(username.length() > 24) return;

	client->setName(username);
	if(client->connected()){
		client->doName();
	}
}

// General funcs
void ParupaintWindow::addChatMessage(const QString & msg, const QString & usr)
{
	chat->show();
	chat->AddMessage(msg, usr);
	//QApplication::alert(this, 1000);
}

QSize ParupaintWindow::canvasDimensions()
{
	return scene->canvas()->dimensions();
}

QDir ParupaintWindow::saveDir() const
{
	QSettings cfg;

	QString default_path = (QDir::homePath() + "/parupaint/");
#ifdef Q_OS_WIN
		default_path = ".";

#endif
	QFileInfo saved(cfg.value("client/directory", default_path).toString());
	QDir dir = saved.dir();
	if(!dir.exists()) dir.mkpath(dir.path());
	qDebug() << "Save dir:" << dir.path();
	return dir;
}


void ParupaintWindow::setLocalServer(const QString & server)
{
	this->local_server = server;
}
