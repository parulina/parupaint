#ifndef PARUPAINTPANVASINPUTOUTPUT_H
#define PARUPAINTPANVASINPUTOUTPUT_H

#include <QString>

class ParupaintPanvas;
class ParupaintPanvasInputOutput
{
	public:
	static bool savePanvas(ParupaintPanvas * panvas, QString & filename, QString & errorStr);
	static bool saveImage(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool savePPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool exportZIPSequence(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool exportGIF(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool exportAV(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);

	static bool loadPanvas(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadImage(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadGIF(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadORA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
	static bool loadPPA(ParupaintPanvas * panvas, const QString & filename, QString & errorStr);
};

#endif
