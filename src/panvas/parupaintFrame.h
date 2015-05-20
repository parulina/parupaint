#ifndef PARUPAINTFRAME_H
#define PARUPAINTFRAME_H

#include <QImage>
#include <QColor>

class ParupaintFrame {
	private:
	bool 	Extended;
	QImage 	Frame;
	float	Opacity;

	public:
	ParupaintFrame();
	ParupaintFrame(QSize rect, float opacity=0);

	void New(QSize);
	void Resize(QSize);
	QImage GetImage() const;
	void LoadFromData(const QByteArray&);
	
	void ClearColor(QColor);
	//void DrawStep(ParupaintStep * step);
	void DrawStep(float x, float y, float x2, float y2, float width, QColor color);

	void SetOpacity(float);
	void SetExtended(bool);
	bool IsExtended();
	float GetOpacity() const;
	QImage GetFrame();
};

#endif
