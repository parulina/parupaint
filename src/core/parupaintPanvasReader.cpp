
#include <QMap>
#include <QByteArray>
#include <QFile>
#include <QRegExp>
#include <QDir>
#include <QFileInfo>
#include <QDomDocument>
#include <QPainter>

#include <QDebug>

#include "parupaintPanvasReader.h"
#include "panvasTypedefs.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../karchive/KTar"
#include "../karchive/KZip"


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
	auto suffix = file.suffix();
	if(suffix == "ora"){
		return this->LoadOra(path);
	} else if(suffix == "png"){
		return this->LoadPng(path);
	} else if(suffix == "gz" || suffix == "ppa"){
		return this->LoadParupaintArchive(path);
	}

	return PANVAS_READER_RESULT_NOTFOUND;

}

PanvasReaderResult ParupaintPanvasReader::LoadPng(const QString filename)
{
	if(!panvas) return PANVAS_READER_RESULT_ERROR;

	QImage img(filename);
	panvas->Clear();
	panvas->Resize(img.size());
	panvas->SetLayers(1, 1);
	panvas->GetLayer(0)->SetFrames(1);
	panvas->GetLayer(0)->GetFrame(0)->Replace(img);
	
	return PANVAS_READER_RESULT_OK;
}

PanvasReaderResult ParupaintPanvasReader::LoadOra(const QString filename)
{
	if(!panvas) return PANVAS_READER_RESULT_ERROR;

	KZip zip(filename);
	if(!zip.open(QIODevice::ReadOnly)) {
		return PANVAS_READER_RESULT_OPENERROR;
	}

	const KArchiveDirectory * d = zip.directory();

	const auto * mte = d->entry("mimetype");
	if(!mte->isFile()) return PANVAS_READER_RESULT_ERROR;
	const KArchiveFile *mtf = static_cast<const KArchiveFile*>(mte);
	if(mtf->data() != "image/openraster") return PANVAS_READER_RESULT_ERROR;

	// okay, it's an ORA.
	const auto * stacke = d->entry("stack.xml");
	if(!stacke->isFile()) return PANVAS_READER_RESULT_ERROR;
	const KArchiveFile *stf = static_cast<const KArchiveFile*>(stacke);
	
	QDomDocument doc;
	if(!doc.setContent(stf->data(), true)) return PANVAS_READER_RESULT_ERROR;

	QDomElement element = doc.documentElement();
	QSize imagesize(
			element.attribute("w").toInt(),
			element.attribute("h").toInt()
			);

	if(imagesize.isEmpty()) return PANVAS_READER_RESULT_ERROR;

	//okay, load the images
	
	// Oras can't be animation so we don't need
	// a special filtering thing
	panvas->Clear();
	panvas->Resize(imagesize);

	const QDomElement stackroot = doc.documentElement().firstChildElement("stack");
	for(auto l = 0; l < stackroot.childNodes().count(); l++){
		QDomElement e = stackroot.childNodes().at(l).toElement();
		if(e.isNull()) continue;
		if(e.tagName() != "layer") continue;
		
		const QString src = e.attribute("src");
		const auto x = e.attribute("x").toDouble();
		const auto y = e.attribute("y").toDouble();


		const auto * layer_entry = d->entry(src);
		if(!layer_entry->isFile()) continue;
		const KArchiveFile *layer_file = static_cast<const KArchiveFile*>(layer_entry);

		QByteArray byte_array = layer_file->data();

		QImage original_pic;
		original_pic.loadFromData(byte_array);

		QImage pic(imagesize, original_pic.format());
		pic.fill(0);
		QPainter paint(&pic);

		auto rp = original_pic.rect();
		auto rr = rp;
		rr.adjust(x, y, x, y);
		paint.drawImage(rr, original_pic, rp);
		
		paint.end();

		panvas->AddLayers(0, 1, 1);
		panvas->GetLayer(0)->GetFrame(0)->Replace(pic);


	}
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
		_fint ff;
		frameLoad(){};
		frameLoad(const QByteArray &ba, int e, _fint f) : frameLoad() {
			bytes = ba; extended = e; ff = f;
		};
	};

	const KArchiveDirectory * d = tar.directory();
	QMap<_lint, QList<frameLoad>> real_frames;
	int width = 0, height = 0;

	if(d->entry("info2.ini")){
		// set -> layer -> frame
		QMap<int, QMap<_lint, QMap<_fint, frameLoad>>> frames;

		foreach(auto file, d->entries()){
			const KArchiveEntry * entry = d->entry(file);
			if(entry->isDirectory()){
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

						frames[ss][ll][ff] = frameLoad(frame->data(), e, ff);
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

		for(auto s = frames.constBegin(); s != frames.constEnd(); ++s){
			for(auto l = s.value().constBegin(); l != s.value().constEnd(); ++l){
				real_frames.insert(real_frames.size(), l.value().values());
			}
		}
	} else {
		auto * f = dynamic_cast<const KArchiveFile*>(d->entry("pp3info.txt"));
		if(f){
			const QByteArray data = f->data();
			QRegExp exp("(\\d+)x(\\d+)");
			if(exp.indexIn(data) != -1) {
				width = exp.cap(1).toInt();
				height = exp.cap(2).toInt();
				qDebug() << width << "x" << height;
			}
		}

		// pre alloc layer slots
		foreach(auto l, d->entries()){
			if(d->entry(l)->isDirectory()) real_frames.insert(real_frames.size(), {});
		}
		
		foreach(auto l, d->entries()){
			if(!d->entry(l)->isDirectory()) continue;
			auto * layer = dynamic_cast<const KArchiveDirectory*>(d->entry(l));

			foreach(auto f, layer->entries()){
				if(!layer->entry(f)->isFile()) continue;
				auto * frame = dynamic_cast<const KArchiveFile*>(layer->entry(f));

				_lint ll = l.toInt();
				QString name = f.section(".", 0, 0);
				_fint ff = name.split("-")[0].toInt();

				int e = 0;
				if(name.indexOf("-") != -1){ e = name.split("-")[1].toInt(); }

				real_frames[ll].append(frameLoad(frame->data(), e, ff));
			}
		}
	}

	// now to modify the panvas
	panvas->Clear();
	panvas->Resize(QSize(width, height));
	panvas->SetLayers(real_frames.size());
	for(auto l = real_frames.begin(); l != real_frames.end(); ++l){
		ParupaintLayer * layer = panvas->GetLayer(l.key());
		layer->SetFrames(l.value().length());
		
		for(auto it = l.value().begin(); it != l.value().end(); ++it){
			auto f = *it;

			ParupaintFrame * frame = layer->GetFrame(f.ff);
			frame->LoadFromData(f.bytes);
			if(f.extended){
				layer->ExtendFrame(f.ff, f.extended);
			}
		}
		
	}


	return PANVAS_READER_RESULT_OK;

}

