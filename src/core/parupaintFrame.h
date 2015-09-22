#ifndef PARUPAINTFRAME_H
#define PARUPAINTFRAME_H

#include <QImage>
#include <QColor>

class ParupaintStroke;

class ParupaintFrame {
	private:
	bool 	Extended;
	QImage 	Frame;
	float	Opacity;

	public:
	~ParupaintFrame();
	ParupaintFrame();
	ParupaintFrame(QSize rect, float opacity=1);

	void New(QSize);
	void Resize(QSize);
	QImage GetImage() const;
	void LoadFromData(const QByteArray&);
	void Replace(QImage);
	
	void ClearColor(QColor);
	void DrawStep(float x, float y, float x2, float y2, float width, QColor color);
	void DrawStep(float x, float y, float x2, float y2, QPen &);
	void Fill(int x, int y, QColor);

	void SetOpacity(float);
	void SetExtended(bool);
	bool IsExtended() const;
	float GetOpacity() const;
};

inline QColor HexToColor(const QString & str)
{
	const QString ahex = str.right(2);
	QColor color(str.left(7));
	if(ahex.length() == 2){
		color.setAlpha(ahex.toInt(nullptr, 16));
	}
	return color;
}


#endif
