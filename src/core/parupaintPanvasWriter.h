#ifndef PARUPAINTPANVASWRITER_H
#define PARUPAINTPANVASWRITER_H

class ParupaintPanvas;

enum PanvasWriterResult{
	PANVAS_WRITER_RESULT_OK,
	PANVAS_WRITER_RESULT_WRITEERROR,
	PANVAS_WRITER_RESULT_ERROR
};

// TODO actually explain to users what save/export means.
// Save
//   ORA
//   PPA (PARUPAINT PROJECT, TAR.GZ renamed)
// Export
//   PNG
//   PNG SEQ
//   WEBP ANIMATION

class ParupaintPanvasWriter
{
	private:
	ParupaintPanvas * panvas;
	public:
	ParupaintPanvasWriter(ParupaintPanvas *);
	void SetPanvas(ParupaintPanvas * );

	PanvasWriterResult Save(const QString, const QString);

	PanvasWriterResult SaveOra(const QString);
	PanvasWriterResult SavePpa(const QString);

	PanvasWriterResult ExportPng(const QString);
	PanvasWriterResult ExportWebp(const QString);
	PanvasWriterResult ExportPngSeq(const QString);
	
};

#endif
