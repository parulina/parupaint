#include "parupaintPanvasInputOutput.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QBuffer>
#ifdef QT_XML_LIB
#include <QDomDocument>
#endif

#include "parupaintPanvas.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../bundled/karchive/KTar"
#include "../bundled/karchive/KZip"

#ifndef PARUPAINT_NOFFMPEG
#include "parupaintAVWriter.h"
#endif

// todo return new filename instead of bool?
bool ParupaintPanvasInputOutput::savePanvas(ParupaintPanvas * panvas, QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QFileInfo file(filename);

	if(file.fileName().section(".", 0, 0).isEmpty()){
		QDateTime time = QDateTime::currentDateTime();
		file.setFile(("drawing_at_" + time.toString("yyyy-MM-dd_HH.mm.ss") + "." + file.completeSuffix()));
	}
	filename = file.fileName();

	if(!file.dir().mkpath(file.absolutePath()))
		return (errorStr = "Couldn't create path.").isEmpty();
	// mkpath returns true if it already exists

	const QString filepath = file.filePath();
	if(filepath.endsWith(".png")){
		return ParupaintPanvasInputOutput::savePNG(panvas, file.absoluteFilePath(), errorStr);
	} else if(filepath.endsWith(".ppa")){
		return ParupaintPanvasInputOutput::savePPA(panvas, file.absoluteFilePath(), errorStr);
	} else {
		return ParupaintPanvasInputOutput::exportAV(panvas, file.absoluteFilePath(), errorStr);
	}
}
bool ParupaintPanvasInputOutput::savePNG(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

	QImage png(panvas->dimensions(), QImage::Format_ARGB32);
	for(int l = 0; l < panvas->layerCount(); l++){
		ParupaintLayer * layer = panvas->layerAt(l);

		// just skip if there isn't, there's no point
		if(!layer->frameCount()) continue;
		
		ParupaintFrame * frame = layer->frameAt(0);
		if(frame){
			QPainter paint(&png);
			paint.drawImage(QPointF(0, 0), frame->renderedImage());
		}
	}
	if(!png.save(filename))
		return (errorStr = "PNG save failed").isEmpty();
	return true;
}
bool ParupaintPanvasInputOutput::savePPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

	KTar tar(filename);
	if(!tar.open(QIODevice::WriteOnly))
		return (errorStr = "Failed open PPA for writing").isEmpty();

	QSize dim = panvas->dimensions();

	// TODO use a json thing
	const QString str_info = QString("%1x%2").arg(dim.width()).arg(dim.height());
	tar.writeFile("pp3info.txt", str_info.toUtf8());

	for(int l = 0; l < panvas->layerCount(); l++){
		ParupaintLayer * layer = panvas->layerAt(l);

		for(int f = 0; f < layer->frameCount(); f++){
			if(!layer->isFrameReal(f)) continue;

			ParupaintFrame * frame = layer->frameAt(f);

			QImage image = frame->image();
			QByteArray byte_array(image.byteCount(), 0);
			QBuffer buf(&byte_array);
			buf.open(QIODevice::WriteOnly);
			image.save(&buf, "png");
			buf.close();

			QString temp_name = QString("%1/%2").arg(l).arg(f);
			if(layer->isFrameExtended(f)){
				temp_name.append(layer->frameLabel(f));

			}
			temp_name += ".png";

			tar.writeFile(temp_name, byte_array);
		}
	}
	tar.close();

	return true;
}
bool ParupaintPanvasInputOutput::exportAV(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

#ifndef PARUPAINT_NOFFMPEG
	QSize dim = panvas->dimensions();
	ParupaintAVWriter writer(filename, dim.width(), dim.height(), panvas->frameRate());

	if(writer.hasError())
		return (errorStr = writer.errorStr()).isEmpty();

	// FIXME canvas mergedFrames clip to canvas rect?
	for(int f = 0; f < panvas->totalFrameCount(); f++){
		QImage img(dim, QImage::Format_ARGB32);
		for(int l = 0; l < panvas->layerCount(); l++){
			ParupaintLayer * layer = panvas->layerAt(l);
			if(!layer) continue;

			ParupaintFrame * frame = layer->frameAt(f);
			if(!frame) continue;

			const QImage & fimg = frame->renderedImage();
			QPainter paint(&img);
			paint.drawImage(img.rect(), fimg, fimg.rect());
		}
		writer.addFrame(img);
	}
	if(writer.hasError())
		return (errorStr = writer.errorStr()).isEmpty();

	return true;
#endif
	return (errorStr = "Export not available.").isEmpty();
}

bool ParupaintPanvasInputOutput::loadPanvas(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QFileInfo file(filename);
	if(!file.exists())
		return (errorStr = "File " + filename + " doesn't exist.").isEmpty();

	const QString filepath = file.filePath();
	if(filepath.endsWith(".png")){
		return ParupaintPanvasInputOutput::loadPNG(panvas, filepath, errorStr);
	} else if(filepath.endsWith(".ora")){
		return ParupaintPanvasInputOutput::loadORA(panvas, filepath, errorStr);
	} else if(filepath.endsWith(".ppa")){
		return ParupaintPanvasInputOutput::loadPPA(panvas, filepath, errorStr);
	}
	return false;
}
bool ParupaintPanvasInputOutput::loadPNG(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QImage img(filename);
	if(img.isNull())
		return (errorStr = "Couldn't load PNG").isEmpty();

	panvas->clearCanvas();
	panvas->resize(img.size());
	panvas->newCanvas(1, 1);
	panvas->layerAt(0)->frameAt(0)->replaceImage(img.convertToFormat(QImage::Format_ARGB32));
	return true;
}
bool ParupaintPanvasInputOutput::loadORA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

#ifdef QT_XML_LIB

	KZip zip(filename);
	if(!zip.open(QIODevice::ReadOnly))
		return (errorStr = "Couldn't read ORA").isEmpty();

	const KArchiveDirectory * d = zip.directory();

	const auto * mte = d->entry("mimetype");
	if(!mte->isFile())
		return (errorStr = "Is not a file...").isEmpty();
	const KArchiveFile *mtf = static_cast<const KArchiveFile*>(mte);
	if(mtf->data() != "image/openraster")
		return (errorStr = "File is not ORA.").isEmpty();

	// okay, it's an ORA.
	const auto * stacke = d->entry("stack.xml");
	if(!stacke->isFile())
		return (errorStr = "Corrupted ORA?").isEmpty();
	const KArchiveFile *stf = static_cast<const KArchiveFile*>(stacke);
	
	QDomDocument doc;
	if(!doc.setContent(stf->data(), true))
		return (errorStr = "doc.setContent error").isEmpty();

	QDomElement element = doc.documentElement();
	QSize imagesize(element.attribute("w").toInt(),
			element.attribute("h").toInt());

	if(imagesize.isEmpty())
		return (errorStr = "Invalid size.").isEmpty();

	//okay, load the images
	
	// Oras can't be animation so we don't need
	// a special filtering thing
	panvas->clearCanvas();
	panvas->resize(imagesize);

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

		ParupaintLayer * layer = new ParupaintLayer(panvas);
		layer->insertFrame(new ParupaintFrame(pic, layer));
		panvas->insertLayer(layer);

	}
	if(true) return true;
#endif
	return (errorStr = "unable to load ora files").isEmpty();
}
bool ParupaintPanvasInputOutput::loadPPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	KTar tar(filename);
	if(!tar.open(QIODevice::ReadOnly))
		return (errorStr = "Couldn't read PPA").isEmpty();


	struct frameLoad {
		QByteArray bytes;
		int extended;
		int ff;
		frameLoad(){};
		frameLoad(const QByteArray &ba, int e, int f) : frameLoad() {
			bytes = ba; extended = e; ff = f;
		};
	};

	const KArchiveDirectory * d = tar.directory();
	QMap<int, QList<frameLoad>> real_frames;
	int width = 0, height = 0;

	if(d->entry("info2.ini")){
		// set -> layer -> frame
		QMap<int, QMap<int, QMap<int, frameLoad>>> frames;

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
						int ll = l.toInt();

						QString name = f.split(".")[0];
						int ff = name.split("-")[0].toInt();

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
			return (errorStr = "Invalid width/height").isEmpty();
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

				int ll = l.toInt();
				QString name = f.section(".", 0, 0);
				int ff = name.split("-")[0].toInt();

				int e = 0;
				if(name.indexOf("-") != -1){ e = name.split("-")[1].toInt(); }

				real_frames[ll].append(frameLoad(frame->data(), e, ff));
			}
		}
	}

	// now to modify the panvas
	panvas->clearCanvas();
	panvas->resize(QSize(width, height));
	for(int i = 0; i < real_frames.size(); i++){
		panvas->insertLayer(new ParupaintLayer(panvas));
	}
	for(auto l = real_frames.begin(); l != real_frames.end(); ++l){
		ParupaintLayer * layer = panvas->layerAt(l.key());
		for(int i = 0; i < l.value().length(); i++){
			layer->insertFrame(new ParupaintFrame(panvas->dimensions()));
		}
		
		for(auto it = l.value().begin(); it != l.value().end(); ++it){
			auto f = *it;

			QImage img;
			img.loadFromData(f.bytes);

			ParupaintFrame * frame = layer->frameAt(f.ff);
			frame->replaceImage(img);
			if(f.extended){
				for(int i = 0; i < f.extended; i++){
					layer->extendFrame(f.ff);
				}
			}
		}
		
	}
	return true;
}
