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

	void insertFrame(const QSize &, ParupaintFrame * at);
	void insertFrame(const QSize &, int i);
	void insertFrame(ParupaintFrame* f, ParupaintFrame* at);
	void insertFrame(ParupaintFrame* f, int i = -1);
	void appendFrame(ParupaintFrame* f);

	void removeFrame(ParupaintFrame* f);
	void removeFrame(int i);
	void extendFrame(ParupaintFrame* f);
	void extendFrame(int i);
	void redactFrame(ParupaintFrame* f);
	void redactFrame(int i);
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
