
#include <QKeyEvent>
#include <QEvent>
#include <QSettings>
#include <QShortcut>

#include "parupaintWindow.h"

#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"

#include "overlay/parupaintChat.h"
#include "overlay/parupaintFlayerList.h"
#include "overlay/parupaintColorPicker.h"
#include "overlay/parupaintUserList.h"
#include "overlay/parupaintInfobar.h"

#include <QDockWidget>
#include <QDebug>

ParupaintWindow::ParupaintWindow() : QMainWindow(), 
	// overlay keys
	OverlayKeyShow(Qt::Key_Tab), OverlayKeyHide(Qt::Key_Tab + Qt::SHIFT), OverlayButtonDown(false)
{
	auto * view = new ParupaintCanvasView(this);
	setCentralWidget(view);
	auto * canvas = new ParupaintCanvasPool(view);
	view->SetCanvas(canvas);

	chat = 	new ParupaintChat(this);
	flayer= new ParupaintFlayerList(this);
	picker= new ParupaintColorPicker(this);


	OverlayKeyShow = QKeySequence(Qt::Key_Tab);
	OverlayKeyHide = QKeySequence(Qt::Key_Tab + Qt::SHIFT);

	QShortcut * TabKey = new QShortcut(OverlayKeyShow, this);
	QShortcut * TabKeyShift = new QShortcut(OverlayKeyHide, this);
	connect(TabKey, SIGNAL(activated()), this, SLOT(OverlayKey()));
	connect(TabKeyShift, SIGNAL(activated()), this, SLOT(OverlayKey()));


	UpdateTitle();
	
	QSettings cfg;
	restoreGeometry(cfg.value("mainWindowGeometry").toByteArray());
	restoreState(cfg.value("mainWindowState").toByteArray());

	show();
}

void ParupaintWindow::OverlayKey()
{
	QShortcut* shortcut = qobject_cast<QShortcut*>(sender());
	QKeySequence seq = shortcut->key();
	
	if(seq == OverlayKeyShow){

		if(!OverlayButtonDown) OverlayButtonDown = true;
		else {
			// perm
			ShowOverlay(true);
		}
	} else if(seq == OverlayKeyHide){
		HideOverlay();
	}
}

void ParupaintWindow::ShowOverlay(bool permanent)
{
	if(permanent) {
		chat->show();
		flayer->show();
		picker->show();
	}
}

void ParupaintWindow::HideOverlay()
{
	chat->hide();
	flayer->hide();
	picker->hide();
}

void ParupaintWindow::closeEvent(QCloseEvent * event)
{
	QSettings cfg;
	cfg.setValue("mainWindowGeometry", saveGeometry());
	cfg.setValue("mainWindowState", saveState());
}

void ParupaintWindow::resizeEvent(QResizeEvent* event)
{
	auto w1 = this->width() - chat->width();
	chat->move(w1, 0);

	auto h2 = this->height() - flayer->height();
	flayer->move(0, h2);
	flayer->resize(this->width(), flayer->height());


	QMainWindow::resizeEvent(event);
}


void ParupaintWindow::UpdateTitle()
{
	setWindowTitle(QString("parupaint"));
}

