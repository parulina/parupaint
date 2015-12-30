#include "parupaintFileDialog.h"

#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>

ParupaintFileDialog::ParupaintFileDialog(ParupaintFileDialogType type, QWidget * parent) : 
	QFileDialog(parent, "browse...")
{	
	QSettings cfg;

	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setOption(QFileDialog::DontUseNativeDialog);
	this->setViewMode(QFileDialog::Detail);
	this->setSidebarUrls(QList<QUrl>{});

	QString config_key;
	if(type == dialogTypeOpen){
		this->setWindowTitle("open...");
		this->setFileMode(QFileDialog::ExistingFile);
		this->setNameFilter("supported files (*.jpg *.png *.ora *.ppa)");
		config_key = "lastopen";
	}
	if(type == dialogTypeSaveAs){
		this->setWindowTitle("save as...");
		this->setFileMode(QFileDialog::AnyFile);
		this->setNameFilter("*.jpg *.png *.ppa");
		config_key = "lastsaveas";
	}
	// set sidebar url
	QDir dir(cfg.value("client/directory", QCoreApplication::applicationDirPath()).toString());

	this->setSidebarUrls(QList<QUrl>{
		dir.absolutePath()
	});

	this->restoreState(cfg.value("window/" + config_key).toByteArray());
	QFileInfo lastfile(cfg.value("client/" + config_key, ".").toString());
	this->selectFile(lastfile.absoluteFilePath());

	connect(this, &QFileDialog::fileSelected, this,
	[this, config_key](const QString & file){

		QSettings cfg;
		cfg.setValue("client/" + config_key, file);
		cfg.setValue("window/" + config_key, this->saveState());
	});

	this->show();
}
