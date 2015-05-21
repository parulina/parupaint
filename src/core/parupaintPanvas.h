#ifndef PARUPAINTPANVAS_H
#define PARUPAINTPANVAS_H


#include <QList>
#include <QString>
#include <QRect>
#include <QImage>

#include "panvasTypedefs.h"

class ParupaintLayer;

struct PanvasProjectInformation {
	QString	Name;
	float	Framerate;
	QSize 	Dimensions;

	PanvasProjectInformation();

};

class ParupaintPanvas {
	QList<ParupaintLayer*> 		Layers;
	PanvasProjectInformation 	Info;

	public:
	ParupaintPanvas();
	ParupaintPanvas(QSize dim, _lint l = 1, _fint f = 1);

	void New(QSize dim, _lint l = 1, _fint f = 1);
	void Resize(QSize dim);
	void Clear();

	void SetLayers(_lint l, _lint = 1);
	void AddLayers(_lint l, _lint = 1);
	void RemoveLayers(_lint l, _lint = 1);
	ParupaintLayer * GetLayer(_lint l);
	
	_fint GetTotalFrames();
	_lint GetNumLayers();

	QList<QImage> GetImageFrames();

	QSize GetDimensions() const;
	QSize GetSize() const;
	int GetWidth() const;
	int GetHeight() const;


};

#endif

