
#include <QDir>
#include <QPainter>

#include "parupaintPanvasWriter.h"

#include "parupaintPanvas.h"
#include "parupaintLayer.h"
#include "parupaintFrame.h"

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
	auto suffix = file.completeSuffix();
	if(suffix == "png"){
		return this->ExportPng(path);

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

PanvasWriterResult ParupaintPanvasWriter::SavePpa(const QString)
{
	if(!panvas) return PANVAS_WRITER_RESULT_ERROR;

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



