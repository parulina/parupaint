#ifndef PARUPAINTPANVASREADER_H
#define PARUPAINTPANVASREADER_H

#include "parupaintPanvas.h"
#include <QString>

enum PanvasReaderResult{
	PANVAS_READER_RESULT_OK,
	PANVAS_READER_RESULT_NOTFOUND,
	PANVAS_READER_RESULT_OPENERROR,
	PANVAS_READER_RESULT_ERROR
};

class ParupaintPanvasReader
{
	private:
	ParupaintPanvas * panvas;
	public:
	ParupaintPanvasReader(ParupaintPanvas *);

	void SetPanvas(ParupaintPanvas * );

	PanvasReaderResult Load(const QString directory, QString filename);
	PanvasReaderResult LoadOra(const QString filename);
	PanvasReaderResult LoadPng(const QString filename);
	PanvasReaderResult LoadParupaintArchive(const QString filename);

};

#endif
