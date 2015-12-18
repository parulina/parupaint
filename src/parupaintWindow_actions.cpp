#include "parupaintWindow.h"

#include <QSettings>

#include "dialog/parupaintConnectionDialog.h"
#include "dialog/parupaintFileDialog.h"
#include "dialog/parupaintNewDialog.h"
#include "dialog/parupaintSettingsDialog.h"

void ParupaintWindow::showOpenDialog()
{
	ParupaintFileDialog * dialog = new ParupaintFileDialog(ParupaintFileDialogType::dialogTypeOpen, this);
	connect(dialog, &ParupaintFileDialog::fileSelected, this, &ParupaintWindow::doOpen);

	dialog->activateWindow();
}
void ParupaintWindow::showSaveAsDialog()
{
	ParupaintFileDialog * dialog = new ParupaintFileDialog(ParupaintFileDialogType::dialogTypeSaveAs, this);
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

	dialog->activateWindow();
}

