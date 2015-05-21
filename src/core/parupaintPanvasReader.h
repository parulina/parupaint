#ifndef PARUPAINTPANVASREADER_H
#define PARUPAINTPANVASREADER_H

#include "parupaintPanvas.h"
#include <QString>

class ParupaintPanvasReader
{
	private:
	ParupaintPanvas * panvas;
	public:
	ParupaintPanvasReader(ParupaintPanvas *);

	void SetPanvas(ParupaintPanvas * );

	bool LoadOra(const QString filename);
	bool LoadParupaintArchive(const QString filename);

};

#endif
