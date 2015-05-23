
#include <QMap>
#include <QByteArray>
#include <QFile>
#include <QRegExp>
#include <QDir>
#include <QFileInfo>


#include "parupaintPanvasReader.h"
#include "panvasTypedefs.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../karchive/KTar"


ParupaintPanvasReader::ParupaintPanvasReader(ParupaintPanvas * p)
{
	SetPanvas(p);
}

void ParupaintPanvasReader::SetPanvas(ParupaintPanvas * p)
{
	panvas = p;
}


PanvasReaderResult ParupaintPanvasReader::Load(const QString directory, const QString filename)
{
	if(!panvas) return PANVAS_READER_RESULT_ERROR;
	QDir dir(directory);
	if(!dir.exists()) return PANVAS_READER_RESULT_NOTFOUND;

	QFileInfo file(dir, filename);
	if(!file.exists()) return PANVAS_READER_RESULT_NOTFOUND; 

	// prevent going up
	if(file.absoluteFilePath().indexOf(directory) == -1) return PANVAS_READER_RESULT_NOTFOUND;


	auto path = file.filePath();
	auto suffix = file.completeSuffix();
	if(suffix == "ora"){
		return this->LoadOra(path);
	} else if(suffix == "tar.gz"){
		return this->LoadParupaintArchive(path);
	}

	return PANVAS_READER_RESULT_NOTFOUND;

}

PanvasReaderResult ParupaintPanvasReader::LoadOra(const QString filename)
{
	if(!panvas) return PANVAS_READER_RESULT_ERROR;


	return PANVAS_READER_RESULT_OK;
}


PanvasReaderResult ParupaintPanvasReader::LoadParupaintArchive(const QString filename)
{
	if(!panvas) return PANVAS_READER_RESULT_ERROR;
	

	KTar tar(filename);
	if(!tar.open(QIODevice::ReadOnly)) {
		return PANVAS_READER_RESULT_OPENERROR;
	}

	struct frameLoad {
		QByteArray bytes;
		int extended;
		frameLoad(){};
		frameLoad(const QByteArray &ba, int e) : frameLoad() { bytes = ba; extended = e;};
	};

	// set -> layer -> frame
	QMap<int, QMap<_lint, QMap<_fint, frameLoad>>> frames;
	int width = 0;
	int height = 0;
	const KArchiveDirectory * d = tar.directory();

	foreach(auto file, d->entries()){
		const KArchiveEntry * entry = d->entry(file);
		if(entry->isDirectory()){
			// set
			auto * set = dynamic_cast<const KArchiveDirectory*>(entry);
			foreach(auto l, set->entries()){
				if(!set->entry(l)->isDirectory()) continue;
				auto * layer = dynamic_cast<const KArchiveDirectory*>(set->entry(l));

				foreach(auto f, layer->entries()){
					if(!layer->entry(f)->isFile()) continue;
					auto * frame = dynamic_cast<const KArchiveFile*>(layer->entry(f));

					int ss = file.toInt();
					_lint ll = l.toInt();

					QString name = f.split(".")[0];
					_lint ff = name.split("-")[0].toInt();

					int e = 0;
					if(name.indexOf("-") != -1){
						e = name.split("-")[1].toInt();
					}

					frames[ss][ll][ff] = frameLoad(frame->data(), e);
				}


			}
		} else if(entry->isFile()) {
			auto * f = dynamic_cast<const KArchiveFile*>(entry);
			if(file == "info2.ini") {
				const QByteArray fbytes = f->data();

				QRegExp exp("CanvasWidth=(\\d+)");
				if(exp.indexIn(fbytes) != -1) { width = exp.cap(1).toInt(); }
				exp.setPattern("CanvasHeight=(\\d+)");
				if(exp.indexIn(fbytes) != -1) { height = exp.cap(1).toInt(); }
			}
		}
	}
	if(!width || !height) {
		return PANVAS_READER_RESULT_ERROR;
	}

	QMap<_lint, QList<frameLoad>> real_frames;
	for(auto s = frames.constBegin(); s != frames.constEnd(); ++s){
		// s = set
		for(auto l = s.value().constBegin(); l != s.value().constEnd(); ++l){
			real_frames.insert(real_frames.size(), l.value().values());
// 			qDebug() << s.key() << "," << l.key() << "added as" << real_frames.size()-1;
		}
	}

	// now to modify the panvas
	panvas->Clear();
	panvas->Resize(QSize(width, height));
	panvas->SetLayers(real_frames.size());
	for(auto l = real_frames.begin(); l != real_frames.end(); ++l){
		ParupaintLayer * layer = panvas->GetLayer(l.key());
		layer->SetFrames(l.value().length());
		
		for(auto f = 0; f < l.value().length(); f++){
			auto ff = l.value().at(f);
			ParupaintFrame * frame = layer->GetFrame(f);
			frame->LoadFromData(ff.bytes);
			// do something with ff.extended
			if(ff.extended){
				layer->ExtendFrame(f, ff.extended);
			}
		}
		
	}


	return PANVAS_READER_RESULT_OK;

}

