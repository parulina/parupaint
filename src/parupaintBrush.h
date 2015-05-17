#ifndef PARUPAINTBRUSH_H
#define PARUPAINTBRUSH_H

#include <QColor>

class ParupaintBrush {

	private:
	QColor 	color;
	QString	name;
	QPointF position;
	float 	width;
	float   pressure;
	

	public:
	ParupaintBrush();

	void SetName(QString);
	void SetColor(QColor);
	void SetPosition(QPointF);
	void SetWidth(float);
	void SetPressure(float);

	QColor GetColor() const;
	QPointF GetPosition() const;
	QString GetName() const;
	float GetWidth() const;
	float GetPressure() const;


};

#endif
