#include "parupaintWindow.h"

#include <QDebug>
#include <QSettings>

#include "parupaintKeys.h"

#include "dialog/parupaintConnectionDialog.h"
#include "dialog/parupaintFileDialog.h"
#include "dialog/parupaintNewDialog.h"
#include "dialog/parupaintSettingsDialog.h"
#include "dialog/parupaintKeyBindsDialog.h"
#include "dialog/parupaintPasswordDialog.h"

void ParupaintWindow::showOpenDialog()
{
	ParupaintFileDialog * dialog = new ParupaintFileDialog(ParupaintFileDialogType::dialogTypeOpen, this);
	connect(dialog, &ParupaintFileDialog::fileSelected, this, &ParupaintWindow::doOpen);

	QSettings cfg;
	if(!cfg.contains("client/lastopen")) dialog->setDirectory(this->saveDir().path());

	dialog->activateWindow();
}
void ParupaintWindow::showSaveAsDialog()
{
	ParupaintFileDialog * dialog = new ParupaintFileDialog(ParupaintFileDialogType::dialogTypeSaveAs, this, this->saveName());

	connect(dialog, &ParupaintFileDialog::fileSelected, [&](const QString & file){
		QFileInfo fi(this->doSaveAs(file));
		this->addChatMessage("Saved as '<a href=\"file:///"+fi.absoluteFilePath()+"\">"+fi.fileName()+"</a>'.");
	});

	dialog->activateWindow();
}

void ParupaintWindow::showNewDialog()
{
	ParupaintNewDialog * dialog = new ParupaintNewDialog(this);
	connect(dialog, &ParupaintNewDialog::NewSignal, this, &ParupaintWindow::doNew);

	QSize canvas_size = this->canvasDimensions();
	dialog->setOriginalDimensions(canvas_size.width(), canvas_size.height());

	dialog->activateWindow();
}

void ParupaintWindow::showConnectDialog()
{
	ParupaintConnectionDialog * dialog = new ParupaintConnectionDialog(this);
	connect(dialog, &ParupaintConnectionDialog::onConnect, this, &ParupaintWindow::doConnect);
	connect(dialog, &ParupaintConnectionDialog::onDisconnect, this, &ParupaintWindow::doDisconnect);

	dialog->activateWindow();
}

void ParupaintWindow::showSettingsDialog()
{
	ParupaintSettingsDialog * dialog = new ParupaintSettingsDialog(this);
	connect(dialog, &ParupaintSettingsDialog::pixelgridChanged, view, &ParupaintCanvasView::setPixelGrid);
	connect(dialog, &ParupaintSettingsDialog::keyBindOpen, this, &ParupaintWindow::showKeyBindDialog);

	// BTW, this does not set the session password on the remote server.
	// only the local one.
	connect(dialog, &ParupaintSettingsDialog::sessionPasswordChanged, this, &ParupaintWindow::doLocalSessionPassword);
	connect(dialog, &ParupaintSettingsDialog::nameChanged, this, &ParupaintWindow::doUserName);

	dialog->activateWindow();
}

void ParupaintWindow::showKeyBindDialog()
{
	ParupaintKeyBindsDialog * dialog = new ParupaintKeyBindsDialog(key_shortcuts, this);
	dialog->activateWindow();
}

void ParupaintWindow::showPasswordDialog()
{
	ParupaintPasswordDialog * dialog = new ParupaintPasswordDialog(this);
	connect(dialog, &ParupaintPasswordDialog::enterPassword, this, &ParupaintWindow::doJoinPassword);
	// TODO knock knock feature?
	//connect(dialog, &ParupaintPasswordDialog::knockKnock, client, &ParupaintClientInstance::doKnock);

	dialog->activateWindow();
}
