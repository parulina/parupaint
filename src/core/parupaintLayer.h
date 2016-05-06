#ifndef PARUPAINTLAYER_H
#define PARUPAINTLAYER_H

#include "parupaintFrame.h"

#include <QObject>
#include <QChar>
#include <QColor>
#include <QList>

int textModeToEnum(const QString & m);

class ParupaintPanvas;

enum ParupaintFrameExtensionsDirection {
	FRAME_NOT_EXTENDED = 0,
	FRAME_EXTENDED_RIGHT,
	FRAME_EXTENDED_LEFT,
	FRAME_EXTENDED_MIDDLE,
};

class ParupaintLayer : public QObject
{
Q_OBJECT
	private:
	QList<ParupaintFrame*> frames;
	bool layer_visible;
	QString layer_name;
	int layer_mode;

	private slots:
	void removeFrameObject(QObject *);

	signals:
	void onVisiblityChange(bool visible);
	void onNameChange(const QString & name);
	void onModeChange(int mode);
	void onContentChange();

	public:
	ParupaintLayer(QObject * = nullptr, const QSize & = QSize(), int frames = 0);

	ParupaintPanvas * parentPanvas();

	void resize(const QSize &);

	bool insertFrame(const QSize &, ParupaintFrame * at);
	bool insertFrame(const QSize &, int i);
	bool insertFrame(ParupaintFrame* f, ParupaintFrame* at);
	bool insertFrame(ParupaintFrame* f, int i = -1);
	bool appendFrame(ParupaintFrame* f);

	bool removeFrame(ParupaintFrame* f);
	bool removeFrame(int i);
	bool extendFrame(ParupaintFrame* f);
	bool extendFrame(int i);
	bool redactFrame(ParupaintFrame* f);
	bool redactFrame(int i);

	bool isFrameExtended(ParupaintFrame* f);
	bool isFrameExtended(int i);
	bool isFrameReal(ParupaintFrame* f);
	bool isFrameReal(int i);

	int frameExtendedDirection(ParupaintFrame * f);
	int frameExtendedDirection(int i);
	QChar frameExtendedChar(ParupaintFrame* f);
	QChar frameExtendedChar(int i);
	QString frameLabel(ParupaintFrame* f);
	QString frameLabel(int i);

	int frameIndex(ParupaintFrame*);
	ParupaintFrame * frameAt(int);

	void setMode(int mode);
	void setMode(const QString & textmode);
	int mode() const;
	QString modeString() const;
	void setName(const QString & name);
	QString name() const;
	void setVisible(bool b);
	bool visible() const;

	int frameCount();
	int realFrameCount();
	QList<QImage> imageFrames(bool rendered = false);
	QList<QImage> renderedImageFrames();
};

#endif
