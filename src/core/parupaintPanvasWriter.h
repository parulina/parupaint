#ifndef PARUPAINTPANVASWRITER_H
#define PARUPAINTPANVASWRITER_H

#include <QString>
class ParupaintPanvas;

enum PanvasWriterResult{
	PANVAS_WRITER_RESULT_OK,
	PANVAS_WRITER_RESULT_WRITEERROR,
	PANVAS_WRITER_RESULT_ERROR
};

class ParupaintPanvasWriter
{
	private:
	ParupaintPanvas * panvas;
	public:
	ParupaintPanvasWriter(ParupaintPanvas *);
	void SetPanvas(ParupaintPanvas * );

	PanvasWriterResult Save(const QString, const QString);

	PanvasWriterResult SaveOra(const QString);
	PanvasWriterResult SaveParupaintArchive(const QString);

	PanvasWriterResult ExportAV(const QString);
	PanvasWriterResult ExportPng(const QString);
	PanvasWriterResult ExportPngZip(const QString);
	PanvasWriterResult ExportPngSeq(const QString);
	
};

#endif
