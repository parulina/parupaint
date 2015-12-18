#ifndef PARUPAINTFRAME_H
#define PARUPAINTFRAME_H

class ParupaintLayer;

#include <QObject>
#include <QImage>
#include <QColor>

class ParupaintFrame : public QObject
{
Q_OBJECT
	private:
	QImage	frame;
	qreal	frame_opacity;

	signals:
	void onResize();
	void onChange(QRect);
	void onOpacityChange();

	public:
	ParupaintFrame(QObject * = nullptr);
	ParupaintFrame(const QSize &, QObject * = nullptr, qreal op = 1.0);
	ParupaintFrame(const QImage &, QObject * = nullptr, qreal op = 1.0);

	ParupaintLayer * parentLayer();

	void resize(const QSize &);
	void clear(const QColor);
	void replaceImage(const QImage & img);
	void setOpacity(qreal op);

	qreal opacity();
	const QImage & image();
	QImage renderedImage();

	QRect drawLine (const QLineF & line, const qreal w, const QColor);
	QRect drawLine (const QLineF & line, const QPen &);
	QRect drawImage(const QPointF & pos, const QImage &);
	QRect drawFill (const QPointF & pos, QColor to_color, QColor from_color = QColor());
};

#endif
