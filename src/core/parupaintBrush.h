#ifndef PARUPAINTBRUSH_H
#define PARUPAINTBRUSH_H

#include "panvasTypedefs.h"

class ParupaintStroke;

#include <QPointF>
#include <QColor>

class ParupaintBrush {

	private:
	QColor 	color;
	QString	name;
	QPointF position;
	float 	width;
	float   pressure;
	_lint	layer;
	_fint	frame;
	bool 	drawing;
	int 	tooltype;


	ParupaintStroke * CurrentStroke;
	ParupaintStroke * LastStroke;
	

	public:
	ParupaintBrush();
	ParupaintBrush(QString, float, QColor);

	QPen ToPen();


	void SetName(QString);
	void SetColor(QColor);
	void SetPosition(QPointF);
	void SetPosition(float, float);
	void SetWidth(float);
	void SetPressure(float);
	void SetLayer(_lint);
	void SetFrame(_fint);
	void SetDrawing(bool);
	void SetCurrentStroke(ParupaintStroke *);
	void SetLastStroke(ParupaintStroke *);
	void SetToolType(int t);

	QColor GetColor() const;
	QString GetColorString() const;
	QPointF GetPosition() const;
	QString GetName() const;
	float GetWidth() const;
	float GetPressure() const;
	_lint GetLayer() const;
	_fint GetFrame() const;
	bool IsDrawing() const;
	ParupaintStroke * GetCurrentStroke() const;
	ParupaintStroke * GetLastStroke() const;
	int GetToolType() const;

};

#endif
