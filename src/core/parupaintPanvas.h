#ifndef PARUPAINTPANVAS_H
#define PARUPAINTPANVAS_H

#include "parupaintLayer.h"

#include <QImage> // GetImageFrames
#include <QString>
#include <QList>
#include <QObject>

class ParupaintLayer;
struct ParupaintPanvasInfo {
	QString 	name;
	qreal 		framerate;
	QSize		dimensions;

	ParupaintPanvasInfo();
};

class ParupaintPanvas : public QObject
{
Q_OBJECT
	protected:
	QList<ParupaintLayer*> 	layers;
	ParupaintPanvasInfo 	info;

	private slots:
	void removeLayerObject(QObject*);

	signals:
	void onCanvasResize(const QSize &);
	void onCanvasChange();
	void onCanvasContentChange();

	public:
	ParupaintPanvas(QObject * = nullptr, const QSize & = QSize(), int layers = 0, int frames = 0);

	void resize(const QSize &);
	void clearCanvas();
	void newCanvas(int l, int f = 0);
	void newCanvas(const QList<ParupaintLayer*> &);

	void insertLayer(int i, int f = 0);
	void insertLayer(ParupaintLayer* l, ParupaintLayer* at);
	void insertLayer(ParupaintLayer* l, int i = -1);
	void appendLayer(ParupaintLayer* l);

	void removeLayer(ParupaintLayer * l);
	void removeLayer(int i);
	int layerIndex(ParupaintLayer*);
	ParupaintLayer * layerAt(int);

	int totalFrameCount();
	int layerCount();
	QList<QImage> mergedImageFrames(bool rendered = false);
	QImage mergedImage(bool rendered = false);

	void setProjectName(const QString &);
	void setFrameRate(qreal = 24);
	const QString & projectName();
	qreal frameRate();
	const QSize & dimensions() const;
};

#endif
