
#include <QDir>
#include <QPainter>
#include <QBuffer>

#include "parupaintPanvasWriter.h"

#include "parupaintPanvas.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

#include "../karchive/KTar"

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
	} else if(suffix == "ppa"){
		return this->SaveParupaintArchive(path);
	} else {
		// image sequence when no other extension
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


PanvasWriterResult ParupaintPanvasWriter::ExportWebp(const QString path)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

	return PANVAS_WRITER_RESULT_OK;
}

PanvasWriterResult ParupaintPanvasWriter::ExportPngSeq(const QString path)
{
	return PANVAS_WRITER_RESULT_ERROR;
}



