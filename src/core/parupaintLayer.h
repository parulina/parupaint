#ifndef PARUPAINTLAYER_H
#define PARUPAINTLAYER_H

#include "panvasTypedefs.h"

#include <QChar>
#include <QColor>
#include <QList>
class ParupaintFrame;
class QSize;

enum FrameExtensionDirection {
	FRAME_NOT_EXTENDED = 0,
	FRAME_EXTENDED_RIGHT,
	FRAME_EXTENDED_LEFT,
	FRAME_EXTENDED_MIDDLE,
};

class ParupaintLayer {
	private:
	QList<ParupaintFrame*>	Frames;
	QSize Dimensions;

	public:

	~ParupaintLayer();
	ParupaintLayer();
	ParupaintLayer(QSize s, _fint = 1);

	void New(QSize s);
	void Resize(QSize s);
	void Clear();
	void Fill(QColor);
	void Fill(_fint f, QColor);

	void SetFrames(_fint);
	void AddFrames(_fint, _fint = 1);
	void RemoveFrames(_fint f, _fint n = 1);
	void ExtendFrame(_fint f, _fint n = 1);
	void RedactFrame(_fint f, _fint n = 1);

	ParupaintFrame * GetFrame(_fint f) const;
	QChar GetFrameChar(_fint f);
	QString GetFrameLabel(_fint f);

	bool IsFrameExtended(_fint);
	bool IsFrameReal(_fint);
	bool IsFrameValid(_fint);

	FrameExtensionDirection GetFrameExtendedDirection(_fint);
	
	_fint GetNumFrames();
	_fint GetNumRealFrames();
};


#endif
