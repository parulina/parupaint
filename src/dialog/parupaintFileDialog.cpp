#include "parupaintFileDialog.h"

#include <QDebug>
#include <QCoreApplication>
#include <QSettings>
#include <QFileInfo>
#include <QGridLayout>
#include <QMimeDatabase>
#include <QPixmapCache>
#include <QStandardPaths>
#include <QCryptographicHash>

#include "../bundled/karchive/KZip"

ParupaintFileDialogPreview::ParupaintFileDialogPreview(QWidget * parent) :
	QLabel(parent)
{
	this->setAlignment(Qt::AlignCenter);
}

void ParupaintFileDialogPreview::setFilePreview(const QString & file)
{
	QPixmap pm;
	QPixmapCache::find(file, &pm);

	if(pm.isNull()){
		// okay, its not in cache, find it in
		// thumbnail dirs
		const QString thumb_dir = QStandardPaths::standardLocations(QStandardPaths::GenericCacheLocation).first();
		if(!thumb_dir.isEmpty()){

			QUrl uri = QUrl::fromLocalFile(file);
			const QByteArray thumb_name = QCryptographicHash::hash(uri.toString().toUtf8(), QCryptographicHash::Md5).toHex();
			QString thumb_path = "thumbnails/normal/" + QString(thumb_name) + ".png";
			QString path = QStandardPaths::locate(QStandardPaths::GenericCacheLocation, thumb_path);

			// at least this works on linux...
			if(!path.isEmpty()){
				pm = QPixmap(thumb_name);
			}
		}
	}
	if(pm.isNull()){
		QMimeDatabase db;
		QString mime = db.mimeTypeForFile(file).name();

		QStringList direct_loadable = {"image/jpeg", "image/png", "image/gif"};
		QStringList zip_thumbnail = {"image/openraster", "application/zip"};

		if(direct_loadable.contains(mime)){
			pm = QPixmap(file);

		} else if(zip_thumbnail.contains(mime)){
			KZip zip(file);
			if(zip.open(QIODevice::ReadOnly)){
				const KArchiveDirectory * dir = zip.directory();
				if(dir){
					const KArchiveEntry * thumb_entry = dir->entry("Thumbnails/thumbnail.png");
					if(thumb_entry){
						const KArchiveFile * thumb_file = static_cast<const KArchiveFile*>(thumb_entry);
						pm.loadFromData(thumb_file->data(), "png");
					}
				}
			}
		}

		if(!pm.isNull()){
			int w = sizeHint().width(), h = sizeHint().height();
			if(pm.width() > w || pm.height() > h){
				pm = pm.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation);
			}
		}
		QPixmapCache::insert(file, pm);
	}

	this->setPixmap(pm);
}

QSize ParupaintFileDialogPreview::minimumSizeHint() const
{
	return this->sizeHint();
}
QSize ParupaintFileDialogPreview::sizeHint() const
{
	return QSize(256, 256);
}


ParupaintFileDialog::ParupaintFileDialog(ParupaintFileDialogType type, QWidget * parent, const QString & selectfile) : 
	QFileDialog(parent, "browse...")
{	
	QSettings cfg;

	this->setAttribute(Qt::WA_DeleteOnClose);
	this->setOption(QFileDialog::DontUseNativeDialog);
	this->setViewMode(QFileDialog::Detail);

	QString config_key;
	if(type == dialogTypeOpen){
		this->setWindowTitle("open...");
		this->setFileMode(QFileDialog::ExistingFile);
		this->setNameFilter("supported files (*.jpg *.png *.gif *.ora *.ppa)");
		this->setAcceptMode(QFileDialog::AcceptOpen);
		config_key = "lastopen";
	}
	if(type == dialogTypeSaveAs){
		this->setWindowTitle("save as...");
		this->setFileMode(QFileDialog::AnyFile);
		this->setNameFilter("*.jpg *.png *.ppa");
		this->setAcceptMode(QFileDialog::AcceptSave);
		this->setDefaultSuffix(".png");
		config_key = "lastsaveas";
	}
	// set sidebar url

	QString default_dir = cfg.value("client/directory").toString();
	QList<QUrl> sidebar = {
		QUrl::fromLocalFile(QCoreApplication::applicationDirPath()),
		QUrl::fromLocalFile(QDir::currentPath()),
		QUrl::fromLocalFile(default_dir)
	};
	this->setSidebarUrls(sidebar);

	this->restoreState(cfg.value("window/" + config_key).toByteArray());

	// if file isn't set by outside, put the file as the last selected one
	QFileInfo lastfile(cfg.value("client/" + config_key, ".").toString());
	if(lastfile.isFile() && (selectfile.isEmpty() && lastfile.fileName() == selectfile)){
		qDebug() << "Settings from lastfile";
		this->setDirectory(lastfile.absoluteDir());
		this->selectFile(lastfile.fileName());
	} else if(!selectfile.isEmpty()) {
		qDebug() << "Settings from selectfile";
		this->selectFile(selectfile);
	} else {
		qDebug() << "No selectfile, default .ppa";
		QFileInfo info(default_dir, ".ppa");
		this->selectFile(info.filePath());
	}

	connect(this, &QFileDialog::fileSelected, this,
	[this, config_key](const QString & file){

		QSettings cfg;
		cfg.setValue("client/" + config_key, file);
		cfg.setValue("window/" + config_key, this->saveState());
	});

	ParupaintFileDialogPreview * preview = new ParupaintFileDialogPreview(this);
	connect(this, &ParupaintFileDialog::currentChanged, preview, &ParupaintFileDialogPreview::setFilePreview);
	preview->setFilePreview(this->selectedFiles().first());

	QGridLayout * layout = qobject_cast<QGridLayout*>(this->layout());
	if(layout){
		layout->setColumnMinimumWidth(1, 500);
		layout->addWidget(preview, 1, 3, 3, 1);
	}

	this->show();
}
