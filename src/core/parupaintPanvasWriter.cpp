
#include <QDir>
#include <QPainter>
#include <QBuffer>

#include "parupaintPanvasWriter.h"

#include "parupaintPanvas.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../bundled/karchive/KTar"
#include "../bundled/karchive/KZip"

#ifndef PARUPAINT_NOFFMPEG
#include "parupaintAVWriter.h"
#endif

#include <QDebug>

ParupaintPanvasWriter::ParupaintPanvasWriter(ParupaintPanvas * p)
{
	this->SetPanvas(p);
}

void ParupaintPanvasWriter::SetPanvas(ParupaintPanvas * p)
{
	panvas = p;
}

PanvasWriterResult ParupaintPanvasWriter::Save(const QString directory, const QString filename)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	QDir dir(directory);
	if(!dir.mkpath(dir.path())) return PANVAS_WRITER_RESULT_ERROR;

	if(!dir.exists()) return PANVAS_WRITER_RESULT_ERROR;

	QFileInfo file(dir, filename);
	auto path = file.filePath();
	auto suffix = file.suffix();
	if(suffix == "png"){
		return this->ExportPng(path);
	//suffix == "apng" && panvas->GetTotalFrames() 
	} else if(suffix == "zip"){
		return this->ExportPngZip(path);
	} else if(suffix == "ppa"){
		return this->SaveParupaintArchive(path);
	} else if(suffix.isEmpty()){
		return this->ExportPngSeq(path);
	} else {
		return this->ExportAV(path);
		// ?
	}

	return PANVAS_WRITER_RESULT_ERROR;

}

PanvasWriterResult ParupaintPanvasWriter::ExportAV(const QString filename)
{
	if(panvas && !filename.isEmpty()){
#ifndef PARUPAINT_NOFFMPEG
		QSize dim = panvas->GetDimensions();
		int fps = 1;

		ParupaintAVWriter writer(filename, dim.width(), dim.height(), fps);
		if(!writer.hasError()){
			for(auto f = 0; f < panvas->GetTotalFrames(); f++){
				QImage img(dim, QImage::Format_ARGB32);
				for(auto l = 0; l < panvas->GetNumLayers(); l++){
					auto * layer = panvas->GetLayer(l);
					if(!layer) continue;

					auto * frame = layer->GetFrame(f);
					if(!frame) continue;

					const QImage & fimg = frame->GetImage();
					QPainter paint(&img);
					paint.drawImage(img.rect(), fimg, fimg.rect());
				}
				writer.addFrame(img);
			}
		} else {
			qDebug() << "Something went wrong!" << writer.errorStr();
		}
		if(!writer.hasError()) return PANVAS_WRITER_RESULT_OK;
		else return PANVAS_WRITER_RESULT_ERROR;
#endif
		//TODO use error strings instead
		return PANVAS_WRITER_RESULT_ERROR;
	}

	return PANVAS_WRITER_RESULT_ERROR;
}

PanvasWriterResult ParupaintPanvasWriter::SaveOra(const QString)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	return PANVAS_WRITER_RESULT_OK;

}

PanvasWriterResult ParupaintPanvasWriter::SaveParupaintArchive(const QString filename)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;
	
	const QString ext = ".png";
	KTar tar(filename);
	if(!tar.open(QIODevice::WriteOnly)){
		return PANVAS_WRITER_RESULT_WRITEERROR;
	}

	auto dim = panvas->GetDimensions();
	auto str_info = QString("%1x%2").arg(dim.width()).arg(dim.height());
	tar.writeFile("pp3info.txt", str_info.toUtf8());

	for(auto l = 0; l < panvas->GetNumLayers(); l++){
		auto * layer = panvas->GetLayer(l);

		for(auto f = 0; f < layer->GetNumFrames(); f++){
			if(!layer->IsFrameReal(f)) continue;

			auto * frame = layer->GetFrame(f);

			auto image = frame->GetImage();
			QByteArray byte_array(image.byteCount(), 0);
			QBuffer buf(&byte_array);
			buf.open(QIODevice::WriteOnly);
			image.save(&buf, "png");
			buf.close();

			QString filename = QString("%1/%2").arg(l).arg(f);
			if(layer->IsFrameExtended(f)){
				auto cc = layer->GetFrameLabel(f);
				filename.append(cc);

			}

			filename += ext;
			tar.writeFile(filename, byte_array);
		}
	}

	tar.close();
	return PANVAS_WRITER_RESULT_OK;

}


PanvasWriterResult ParupaintPanvasWriter::ExportPng(const QString path)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	// This only saves the first frames.
	// gonna have to wait until i compile libapng for this or something
	// ... if i ever bother to
	
	QImage png(panvas->GetDimensions(), QImage::Format_ARGB32);
	for(auto l = 0; l < panvas->GetNumLayers(); l++){
		auto * layer = panvas->GetLayer(l);
		// just skip if there isn't, there's no point
		if(!layer->GetNumFrames()) continue;
		
		auto * frame = layer->GetFrame(0);
		if(frame){
			QPainter paint(&png);
			paint.drawImage(QPointF(0, 0), frame->GetImage());
		}
	}
	if(!png.save(path)) return PANVAS_WRITER_RESULT_ERROR;

	return PANVAS_WRITER_RESULT_OK;
}

PanvasWriterResult ParupaintPanvasWriter::ExportPngSeq(const QString filename)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	QDir dir(filename);
	dir.mkpath(".");

	// clear files
	dir.setNameFilters(QStringList() << "*.*");
	dir.setFilter(QDir::Files);
	foreach(QString dirFile, dir.entryList()) dir.remove(dirFile);

	auto frames = panvas->GetImageFrames();
	for(auto i = 0; i < frames.length(); i++){

		const QImage & image = frames.at(i);
		// Might change this to padded number? for timelapse etc
		QString fname = QString("%1.png").arg(i);
		image.save(QFileInfo(dir, fname).filePath());
	}

	return PANVAS_WRITER_RESULT_OK;
}

PanvasWriterResult ParupaintPanvasWriter::ExportPngZip(const QString filename)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	KZip zip(filename);
	if(!zip.open(QIODevice::WriteOnly)){
		return PANVAS_WRITER_RESULT_WRITEERROR;
	}

	auto frames = panvas->GetImageFrames();
	for(auto i = 0; i < frames.length(); i++){

		const QImage & image = frames.at(i);
		QString fname = QString("%1.png").arg(i);

		QByteArray byte_array(image.byteCount(), 0);
		QBuffer buf(&byte_array);
		buf.open(QIODevice::WriteOnly);
		image.save(&buf, "png");
		buf.close();

		zip.writeFile(fname, byte_array);
	}

	zip.close();
	return PANVAS_WRITER_RESULT_OK;
}



