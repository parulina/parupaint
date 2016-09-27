#include "parupaintPanvasInputOutput.h"

#include <QDebug>
#include <QDir>
#include <QPainter>
#include <QBuffer>
#include <QMovie>
#include <QMimeDatabase>
#include <QImageWriter>

// PPA
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#ifdef QT_XML_LIB
#include <QDomDocument>
#endif

#include "parupaintSnippets.h"
#include "parupaintPanvas.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../bundled/karchive/KZip"

#ifndef PARUPAINT_NOFFMPEG
#include "parupaintAVWriter.h"
#endif

#ifndef PARUPAINT_NOGIF
#include <gif_lib.h>
#include <string.h>
#endif

int BitSize(int n){
	int i;
	for(i = 1; i <= 8; i++){
		if((1 << i) >= n) break;
	}
	return (i);
}

QImage convertToIndexed8(const QImage & img, bool * uses_alpha = nullptr)
{
	QList<QRgb> colors;

	QImage image(img.width(), img.height(), QImage::Format_Indexed8);
	image.fill(0);

	for(int x = 0; x < img.width(); x++){
		for(int y = 0; y < img.height(); y++){
			QRgb c = img.pixel(x, y);
			if(!colors.contains(c)){
				colors << c;
				if(qAlpha(c) == 0 && uses_alpha){
					*uses_alpha = true;
					colors.move(colors.size()-1, 0);
				}
			}
		}
	}
	if(colors.size() > 255) return img.convertToFormat(QImage::Format_Indexed8);
	for(int i = 0; i < colors.size(); i++){
		image.setColor(i, colors[i]);
	}

	for(int x = 0; x < img.width(); x++){
		for(int y = 0; y < img.height(); y++){
			QRgb pix = img.pixel(x, y);
			image.setPixel(x, y, colors.indexOf(pix));
		}
	}
	return image;
}

const QByteArray imageToByteArray(const QImage & image)
{
	QByteArray byte_array(image.byteCount(), 0);
		QBuffer buf(&byte_array);
		buf.open(QIODevice::WriteOnly);
			image.save(&buf, "png");
		buf.close();
	return byte_array;
}

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
	} else if(filepath.endsWith(".gif")){
		return ParupaintPanvasInputOutput::exportGIF(panvas, file.absoluteFilePath(), errorStr);
	} else if(filepath.endsWith(".zip")){
		return ParupaintPanvasInputOutput::exportZIPSequence(panvas, file.absoluteFilePath(), errorStr);
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

	QImage img = panvas->mergedImageFrames(true).first();
	if(!img.save(filename))
		return (errorStr = "Image save failed.").isEmpty();
	return true;
}

bool ParupaintPanvasInputOutput::savePPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

	KZip archive(filename);
	if(!archive.open(QIODevice::WriteOnly))
		return (errorStr = "Failed open PPA for writing").isEmpty();

	archive.writeFile("mimetype", "image/parupaint");

	QImage mergedImage = panvas->mergedImage(true);
	archive.writeFile("Thumbnails/thumbnail.png", imageToByteArray(mergedImage.scaled(256, 256, Qt::KeepAspectRatio, Qt::SmoothTransformation)));
	archive.writeFile("mergedimage.png", imageToByteArray(mergedImage));

	// write image files
	for(int l = 0; l < panvas->layerCount(); l++){
		ParupaintLayer * layer = panvas->layerAt(l);

		for(int f = 0; f < layer->frameCount(); f++){
			if(!layer->isFrameReal(f)) continue;

			ParupaintFrame * frame = layer->frameAt(f);

			QByteArray byte_array = imageToByteArray(frame->image());
			QString frame_name = QString("%1/%2.png").arg(l).arg(layer->frameLabel(f));

			archive.writeFile(frame_name, byte_array);
		}
	}

	// json info
	QJsonObject ppa_info = panvas->json();
	QByteArray json = QJsonDocument(ppa_info).toJson(QJsonDocument::Indented);
	qDebug("%s", qPrintable(json));
	archive.writeFile("pp3.json", json);

	archive.close();

	return true;
}


bool ParupaintPanvasInputOutput::exportZIPSequence(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

	KZip archive(filename);
	if(!archive.open(QIODevice::WriteOnly))
		return (errorStr = "Failed open ZIP sequence for writing").isEmpty();

	int frame = 0;
	foreach(const QImage & image, panvas->mergedImageFrames(true)){
		QByteArray bytes = imageToByteArray(image);
		archive.writeFile(QString("%1/%2_%3.png").arg(QFileInfo(filename).baseName()).arg(QFileInfo(filename).baseName()).arg(frame, 3, 10, QChar('0')), bytes);
		frame++;
	}

	archive.close();

	return true;
}
bool ParupaintPanvasInputOutput::exportGIF(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);
	if(filename.isEmpty())
		return (errorStr = "Enter a filename to save to.").isEmpty();

#ifndef PARUPAINT_NOGIF
	int error = 0;
	GifFileType * gif = EGifOpenFileName(filename.toStdString().c_str(), false, &error);

	foreach(const QImage & image, panvas->mergedImageFrames(true)){

		error = 0;
		bool alpha = false;
		QImage toWrite = convertToIndexed8(image, &alpha);

		QVector<QRgb> colorTable = toWrite.colorTable();
		ColorMapObject cmap;

		int numColors = 1 << BitSize(toWrite.colorCount());
		numColors = 256;

		cmap.ColorCount = numColors;
		cmap.BitsPerPixel = 8;	/// @todo based on numColors (or not? we did ask for Format_Indexed8, so the data is always 8-bit, right?)
		GifColorType* colorValues = (GifColorType*)malloc(cmap.ColorCount * sizeof(GifColorType));
		cmap.Colors = colorValues;
		int c = 0;
		for(; c < toWrite.colorCount(); ++c)
		{
			//qDebug("color %d has %02X%02X%02X", c, qRed(colorTable[c]), qGreen(colorTable[c]), qBlue(colorTable[c]));
			colorValues[c].Red = qRed(colorTable[c]);
			colorValues[c].Green = qGreen(colorTable[c]);
			colorValues[c].Blue = qBlue(colorTable[c]);
		}
		// In case we had an actual number of colors that's not a power of 2,
		// fill the rest with something (black perhaps).
		for (; c < numColors; ++c)
		{
			colorValues[c].Red = 0;
			colorValues[c].Green = 0;
			colorValues[c].Blue = 0;
		}

		/// @todo how to specify which version, or decide based on features in use
		// Because of this call, libgif is not re-entrant
		EGifSetGifVersion(gif, true);

		if ((error = EGifPutScreenDesc(gif, toWrite.width(), toWrite.height(), numColors, 0, &cmap)) == GIF_ERROR)
			qCritical("EGifPutScreenDesc returned error %d", error);

		int fps = (100.0/panvas->frameRate());

		char flags = 1 << 3;
		if(alpha) flags |= 1;

		char graphics_ext[] = {
			flags,
			(char)(fps % 256), (char)(fps / 256),
			(char)(alpha ? 0x00 : 0xff)
		};
		EGifPutExtension(gif, GRAPHICS_EXT_FUNC_CODE, 4, graphics_ext);

		if ((error = EGifPutImageDesc(gif, 0, 0, toWrite.width(), toWrite.height(), 0, &cmap)) == GIF_ERROR)
			qCritical("EGifPutImageDesc returned error %d", error);

		int lc = toWrite.height();
		int llen = toWrite.bytesPerLine();
		for (int l = 0; l < lc; ++l) {
			uchar* line = toWrite.scanLine(l);
			if ((error = EGifPutLine(gif, (GifPixelType*)line, llen)) == GIF_ERROR) {
				qCritical("EGifPutLine returned error %d", error);
			}
		}

		if(true){
			// loop forever
			unsigned char loopblock[3] = {1, 0, 0};
			EGifPutExtensionLeader(gif, APPLICATION_EXT_FUNC_CODE);
			EGifPutExtensionBlock(gif, 11, "NETSCAPE2.0");
			EGifPutExtensionBlock(gif, 3, loopblock);
			EGifPutExtensionTrailer(gif);
		}
	}

	EGifCloseFile(gif, &error);
	return true;
#endif
	return (errorStr = "GIF export not available.").isEmpty();
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
	// ???
	foreach(const QImage & img, panvas->mergedImageFrames(true)){
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

	QMimeType type = QMimeDatabase().mimeTypeForFile(file);
	if(type.name().endsWith("png") || type.name().endsWith("jpg")){
		return ParupaintPanvasInputOutput::loadImage(panvas, file.filePath(), errorStr);
	} else if(type.name().endsWith("gif")){
		return ParupaintPanvasInputOutput::loadGIF(panvas, file.filePath(), errorStr);
	} else if(type.name().endsWith("openraster")){
		return ParupaintPanvasInputOutput::loadORA(panvas, file.filePath(), errorStr);
	} else if(type.name().endsWith("zip")){
		return ParupaintPanvasInputOutput::loadPPA(panvas, file.filePath(), errorStr);
	}
	return (errorStr = "File format not recognized.").isEmpty();
}
bool ParupaintPanvasInputOutput::loadImage(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	QImage img(filename);
	if(img.isNull())
		return (errorStr = "Couldn't load image.").isEmpty();

	panvas->setBackgroundColor(Qt::transparent);

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

	panvas->setBackgroundColor(Qt::transparent);

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
	panvas->setBackgroundColor(Qt::transparent);
	panvas->clearCanvas();
	panvas->resize(imagesize);

	const QDomElement stackroot = doc.documentElement().firstChildElement("stack");
	for(auto l = 0; l < stackroot.childNodes().count(); l++){
		QDomElement e = stackroot.childNodes().at(l).toElement();
		if(e.isNull()) continue;
		if(e.tagName() != "layer") continue;
		
		const QString name = e.attribute("name");
		const QString src = e.attribute("src");
		const qreal opacity = e.attribute("opacity").toDouble();
		const QString composite = e.attribute("composite-op");
		const bool visible = (e.attribute("visibility") == "visible");
		const qreal x = e.attribute("x").toDouble();
		const qreal y = e.attribute("y").toDouble();


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
		layer->setMode(composite);
		layer->setVisible(visible);
		layer->setName(name);

		layer->insertFrame(new ParupaintFrame(pic, layer, opacity));
		panvas->insertLayer(layer, 0);

	}
	if(true) return true;
#endif
	return (errorStr = "unable to load ora files").isEmpty();
}
bool ParupaintPanvasInputOutput::loadPPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr)
{
	Q_ASSERT(panvas);

	KZip archive(filename);
	if(!archive.open(QIODevice::ReadOnly))
		return (errorStr = "Couldn't read PPA").isEmpty();

	const QString pp3("pp3.json");
	const KArchiveDirectory * topdir = archive.directory();

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
	panvas->loadJson(main_obj);

	// load the image frames
	QJsonObject layers = main_obj.value("layers").toObject();
	foreach(const QString & lk, layers.keys()) {

		const QJsonObject & lo = layers.value(lk).toObject();
		const QJsonObject & ff = lo.value("frames").toObject();
		ParupaintLayer * layer = panvas->layerAt(lk.toInt());

		foreach(const QString & fk, ff.keys()){
			const QJsonObject & fo = ff.value(fk).toObject();

			int framenum = fk.toInt();
			if(fk.contains('-')){
				framenum = fk.section('-', 0, 0).toInt();
			}

			const QString fname(QString("%1/%2.png").arg(lk.toInt()).arg(fk));
			const KArchiveFile * img_file = dynamic_cast<const KArchiveFile*>(topdir->entry(fname));
			if(!img_file) continue;

			// load the image
			QImage img(panvas->dimensions(), QImage::Format_ARGB32);
			img.loadFromData(img_file->data());

			ParupaintFrame * frame = layer->frameAt(framenum);
			frame->replaceImage(img);
		}
	}
	return true;
}
