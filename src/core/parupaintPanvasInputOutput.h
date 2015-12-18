#ifndef PARUPAINTPANVASINPUTOUTPUT_H
#define PARUPAINTPANVASINPUTOUTPUT_H

#include <QString>

class ParupaintPanvas;
class ParupaintPanvasInputOutput
{
	public:
	static bool savePanvas(ParupaintPanvas * panvas, QString & filename, QString & errorStr);
	static bool savePNG(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool savePPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool exportAV(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);

	static bool loadPanvas(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadPNG(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadORA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadPPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
};

#endif
