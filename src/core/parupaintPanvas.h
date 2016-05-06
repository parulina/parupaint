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
	QColor		background_color;

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
	void updateLayerObject();

	signals:
	void onCanvasResize(const QSize &);
	void onCanvasBackgroundChange();
	void onCanvasChange();
	void onCanvasContentChange();

	void onCanvasLayerChange(int layer);

	public:
	ParupaintPanvas(QObject * = nullptr, const QSize & = QSize(), int layers = 0, int frames = 0);

	void resize(const QSize &);
	void clearCanvas();
	void newCanvas(int l, int f = 0);
	void newCanvas(const QList<ParupaintLayer*> &);

	bool insertLayer(int i, int f = 0);
	bool insertLayer(ParupaintLayer* l, ParupaintLayer* at);
	bool insertLayer(ParupaintLayer* l, int i = -1);
	bool appendLayer(ParupaintLayer* l);

	bool removeLayer(ParupaintLayer * l);
	bool removeLayer(int i);
	int layerIndex(ParupaintLayer*);
	ParupaintLayer * layerAt(int) const;

	int totalFrameCount();
	int layerCount() const;
	QList<QImage> mergedImageFrames(bool rendered = false);
	QImage mergedImage(bool rendered = false);

	void setProjectName(const QString &);
	void setFrameRate(qreal = 24);
	void setBackgroundColor(const QColor);

	const QString & projectName() const;
	const QString projectDisplayName() const;
	qreal frameRate() const;
	QColor backgroundColor() const;
	const QSize & dimensions() const;

	void loadJson(const QJsonObject & obj);
	QJsonObject json() const;
};

#endif
