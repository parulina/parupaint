#include "parupaintPanvasInputOutput.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QBuffer>
#include <QMovie>

// PPA
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

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
// todo use mimetype instead of endsWidth()?

bool ParupaintPanvasInputOutput::savePanvas(ParupaintPanvas * panvas, QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QFileInfo file(filename);

	if(file.fileName().section(".", 0, 0).isEmpty()){
		QDateTime time = QDateTime::currentDateTime();
		file.setFile(file.dir().path(), "drawing_at_" + time.toString("yyyy-MM-dd_HH.mm.ss") + "." + file.completeSuffix());
	}
	filename = file.fileName();

	if(!file.dir().mkpath(file.absolutePath()))
		return (errorStr = "Couldn't create path.").isEmpty();
	// mkpath returns true if it already exists

	const QString filepath = file.filePath();
	if(filepath.endsWith(".png") || filepath.endsWith(".jpg")){
		return ParupaintPanvasInputOutput::saveImage(panvas, file.absoluteFilePath(), errorStr);
	} else if(filepath.endsWith(".ppa")){
		return ParupaintPanvasInputOutput::savePPA(panvas, file.absoluteFilePath(), errorStr);
	} else {
		return ParupaintPanvasInputOutput::exportAV(panvas, file.absoluteFilePath(), errorStr);
	}
}
bool ParupaintPanvasInputOutput::saveImage(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

	QImage img(panvas->dimensions(), QImage::Format_ARGB32);
	img.fill(0);
	for(int l = 0; l < panvas->layerCount(); l++){
		ParupaintLayer * layer = panvas->layerAt(l);

		// just skip if there isn't, there's no point
		if(!layer->frameCount()) continue;
		
		ParupaintFrame * frame = layer->frameAt(0);
		if(frame){
			QPainter paint(&img);
			paint.drawImage(QPointF(0, 0), frame->renderedImage());
		}
	}
	if(!img.save(filename))
		return (errorStr = "Image save failed.").isEmpty();
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


	QJsonObject layers;
	for(int l = 0; l < panvas->layerCount(); l++){

		ParupaintLayer * layer = panvas->layerAt(l);

		QJsonObject frames_info;
		for(int f = 0; f < layer->frameCount(); f++){
			if(!layer->isFrameReal(f))
				continue;

			ParupaintFrame * frame = layer->frameAt(f);

			QImage image = frame->image();

			// write temp buffer for png
			QByteArray byte_array(image.byteCount(), 0);
			QBuffer buf(&byte_array);
			buf.open(QIODevice::WriteOnly);
			image.save(&buf, "png");
			buf.close();

			QString frame_name = QString("%1/%2.png").arg(l).arg(layer->frameLabel(f));
			frames_info.insert(layer->frameLabel(f), QJsonObject{
				{"opacity", frame->opacity()}
			});
			tar.writeFile(frame_name, byte_array);
		}
		frames_info["visible"] = layer->visible();
		layers.insert(QString::number(l), frames_info);
		// TODO layername
	}

	QJsonObject ppa_info = {
		{"canvasWidth", panvas->dimensions().width()},
		{"canvasHeight", panvas->dimensions().height()},
		{"frameRate", panvas->frameRate()},
		{"projectName", panvas->projectName()},
		{"layers", layers}
	};

	QByteArray json = QJsonDocument(ppa_info).toJson(QJsonDocument::Indented);
	qDebug("%s", qPrintable(json));
	tar.writeFile("pp3.json", json);

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
	if(filepath.endsWith(".png") || filepath.endsWith(".jpg")){
		return ParupaintPanvasInputOutput::loadImage(panvas, filepath, errorStr);
	} else if(filepath.endsWith(".gif")){
		return ParupaintPanvasInputOutput::loadGIF(panvas, filepath, errorStr);
	} else if(filepath.endsWith(".ora")){
		return ParupaintPanvasInputOutput::loadORA(panvas, filepath, errorStr);
	} else if(filepath.endsWith(".ppa")){
		return ParupaintPanvasInputOutput::loadPPA(panvas, filepath, errorStr);
	}
	return (errorStr = "File format not recognized.").isEmpty();
}
bool ParupaintPanvasInputOutput::loadImage(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QImage img(filename);
	if(img.isNull())
		return (errorStr = "Couldn't load image.").isEmpty();

	panvas->clearCanvas();
	panvas->resize(img.size());
	panvas->newCanvas(1, 1);
	panvas->layerAt(0)->frameAt(0)->replaceImage(img.convertToFormat(QImage::Format_ARGB32));
	return true;
}
bool ParupaintPanvasInputOutput::loadGIF(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QMovie gif(filename);
	if(!gif.isValid())
		return (errorStr = "Couldn't load GIF.").isEmpty();
	gif.jumpToFrame(0);

	panvas->clearCanvas();
	panvas->resize(gif.frameRect().size());
	panvas->newCanvas(1, 0);

	do {
		const QImage & gif_image = gif.currentImage().convertToFormat(QImage::Format_ARGB32);
		ParupaintFrame * gif_frame = new ParupaintFrame(gif_image);
		panvas->layerAt(0)->appendFrame(gif_frame);

		gif.jumpToNextFrame();

	} while(gif.currentFrameNumber() != 0);

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

	const QString pp3("pp3.json");
	const KArchiveDirectory * topdir = tar.directory();

	// be sure it has the files
	if(!topdir->entry(pp3))
		return (errorStr = "File is not PPA.").isEmpty();

	const KArchiveFile * json_file = dynamic_cast<const KArchiveFile*>(topdir->entry(pp3));
	if(!json_file)
		return (errorStr = "Couldn't read info file in PPA.").isEmpty();

	QJsonParseError err;
	QJsonDocument info_doc = QJsonDocument::fromJson(json_file->data(), &err);
	if(err.error != QJsonParseError::NoError)
		return (errorStr = "Couldn't read json info: " + err.errorString()).isEmpty();

	QJsonObject main_obj = info_doc.object();
	if(!main_obj.contains("canvasWidth") || !main_obj.contains("canvasHeight"))
		return (errorStr = "Width/height not specified.").isEmpty();

	// load the canvas settings
	panvas->clearCanvas();
	panvas->resize(QSize(main_obj.value("canvasWidth").toInt(180), main_obj.value("canvasHeight").toInt(180)));
	panvas->setFrameRate(main_obj.value("frameRate").toDouble(12));
	panvas->setProjectName(main_obj.value("projectName").toString());

	QJsonObject layers = main_obj.value("layers").toObject();
	foreach(const QString & lk, layers.keys()) {

		const QJsonObject & lo = layers.value(lk).toObject();
		ParupaintLayer * layer = new ParupaintLayer;
		layer->setVisible(lo.value("visible").toBool(true));

		foreach(const QString & fk, lo.keys()){
			if(!lo.value(fk).isObject()) continue;

			const QJsonObject & fo = lo.value(fk).toObject();

			int extended = 0;
			if(fk.contains('-')){
				extended = fk.section('-', 1, 1).toInt();
			}

			const QString fname(QString("%1/%2.png").arg(lk, fk));
			const KArchiveFile * img_file = dynamic_cast<const KArchiveFile*>(topdir->entry(fname));
			if(!img_file) continue;

			// load the image
			QImage img(panvas->dimensions(), QImage::Format_ARGB32);
			img.loadFromData(img_file->data());

			// create a new frame with the image
			ParupaintFrame * frame = new ParupaintFrame(img);
			frame->setOpacity(fo.value("opacity").toDouble(1.0));

			layer->insertFrame(frame, layer->frameCount());
			// extend the frame as far as it wants (TODO: Put limit?)
			for(; extended > 0; extended--){
				layer->extendFrame(frame);
			}
		}
		panvas->insertLayer(layer, panvas->layerCount());
	}
	return true;
}
