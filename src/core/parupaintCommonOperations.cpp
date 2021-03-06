#include "parupaintCommonOperations.h"

// this provides a common set of functions for canvas and brushes

#include <QDebug>
#include "parupaintPanvas.h"
#include "parupaintBrush.h"

// set canvas attributes
bool ParupaintCommonOperations::CanvasAttributeOp(ParupaintPanvas * canvas, const QString & attr, const QVariant & val)
{
	Q_ASSERT(canvas);

	if(attr == "project-name" && val.type() == QVariant::String){
		canvas->setProjectName(val.toString());
		return true;
	}
	if(attr == "project-fps" && val.type() == QVariant::Double){
		canvas->setFrameRate(val.toDouble());
		return true;
	}
	if(attr == "project-bgc" && val.type() == QVariant::Color){
		canvas->setBackgroundColor(val.value<QColor>());
		return true;
	}

	return false;
}

// resize the canvas
bool ParupaintCommonOperations::CanvasResizeOp(ParupaintPanvas * canvas, int w, int h, bool resize)
{
	Q_ASSERT(canvas);

	if(w < 0 || h < 0) return false;
	if(w > 2048*4 || h > 2048*4) return false;

	if(!resize){
		// create a completely new canvas
		canvas->clearCanvas();
		canvas->setBackgroundColor(Qt::white);
		canvas->insertLayer(0, 1);
	}
	canvas->resize(QSize(w, h));

	return true;
}

// paste an image on to a frame
bool ParupaintCommonOperations::LayerFramePasteOp(ParupaintPanvas * canvas, int l, int f, int x, int y, const QImage & img)
{
	Q_ASSERT(canvas);

	if(!img.isNull()){
		ParupaintLayer * layer = canvas->layerAt(l);
		if(layer){
			ParupaintFrame * frame = layer->frameAt(f);
			if(frame){
				frame->drawImage(QPointF(x, y), img);
				return true;
			}
		}
	}
	return false;
}

// fill a frame with a certain color
bool ParupaintCommonOperations::LayerFrameFillOp(ParupaintPanvas * canvas, int l, int f, const QColor & color)
{
	Q_ASSERT(canvas);

	ParupaintLayer * layer = canvas->layerAt(l);
	if(layer){
		ParupaintFrame * frame = layer->frameAt(f);
		if(frame){
			frame->clear(color);
			return true;
		}
	}

	return false;
}

// changes the amount of layers/frames in a canvas
bool ParupaintCommonOperations::LayerFrameChangeOp(ParupaintPanvas * canvas, int l, int f, int lc, int fc, bool extend)
{
	Q_ASSERT(canvas);

	bool changed = false;
	if(lc != 0){
		if(lc < 0 && canvas->layerCount() > 1){
			for(int i = 0; i < -lc; i++)
				if(canvas->removeLayer(l))
					changed = true;
		} else if(lc > 0){
			for(int i = 0; i < lc; i++)
				if(canvas->insertLayer(l, 1))
					changed = true;
		}
	}
	if(fc != 0){
		ParupaintLayer * ff = canvas->layerAt(l);
		if(ff) {
			if(fc < 0 && ff->frameCount() > 1){
				if(extend){
					for(int i = 0; i < -fc; i++)
						if(ff->redactFrame(f))
							changed = true;
				} else {
					for(int i = 0; i < -fc; i++)
						if(ff->removeFrame(f))
							changed = true;
				}
			} else if(fc > 0){
				if(extend){
					for(int i = 0; i < fc; i++)
						if(ff->extendFrame(f))
							changed = true;
				} else {
					for(int i = 0; i < fc; i++)
						if(ff->insertFrame(canvas->dimensions(), f))
							changed = true;
				}
			}
		}
	}
	return changed;
}

// sets an attribute (frame opacity, layer visibility, etc)
bool ParupaintCommonOperations::LayerFrameAttributeOp(ParupaintPanvas * canvas, int l, int f, const QString & attr, const QVariant & val)
{
	Q_ASSERT(canvas);

	ParupaintLayer * layer = canvas->layerAt(l);
	if(layer){
		ParupaintFrame * frame = layer->frameAt(f);
		if(frame){
			if(attr == "frame-opacity" && val.type() == QVariant::Double){
				qreal v = val.toDouble();
				if(v > 1.0) v = 1.0;
				if(v < 0.0) v = 0.0;
				frame->setOpacity(v);
				return true;
			}
		}
		if(attr == "layer-visible" && val.type() == QVariant::Bool){
			layer->setVisible(val.toBool());
			return true;
		}
		if(attr == "layer-name" && val.type() == QVariant::String){
			if(val.toString().length() > 64) return false;

			layer->setName(val.toString());
			return true;
		}
		if(attr == "layer-mode" && (val.type() == QVariant::Double || val.type() == QVariant::Int)){
			layer->setMode(val.toInt());
			return true;
		}
	}

	return false;
}

bool ParupaintCommonOperations::BrushOp(ParupaintBrush * brush, QLineF & line, const QVariantMap & data)
{
	QPointF old_pos = brush->position();

	if(data["x"].type() == QVariant::Double) brush->setX(data["x"].toDouble());
	if(data["y"].type() == QVariant::Double) brush->setY(data["y"].toDouble());
	if(data["s"].type() == QVariant::Double) brush->setSize(data["s"].toDouble());
	if(data["p"].type() == QVariant::Double) brush->setPressure(data["p"].toDouble());
	if(data["c"].type() == QVariant::Color)  brush->setColor(data["c"].value<QColor>());
	if(data["t"].type() == QVariant::Int)    brush->setTool(data["t"].toInt());
	if(data["a"].type() == QVariant::Int)    brush->setPattern(data["a"].toInt());
	if(data["l"].type() == QVariant::Int)    brush->setLayer(data["l"].toInt());
	if(data["f"].type() == QVariant::Int)    brush->setFrame(data["f"].toInt());

	if(data["d"].type() == QVariant::Bool) {

		bool old_d = brush->drawing(),
		     new_d = data["d"].toBool();

		// reset the old position if we have just started drawing.
		if(new_d && !old_d){
			old_pos = brush->position();
		}
		brush->setDrawing(new_d);
	}

	line = QLineF(old_pos, brush->position());
	return true;
}

void ParupaintCommonOperations::AdjustBrush(ParupaintBrush * brush, ParupaintPanvas * canvas)
{
	if(brush->layer() < 0) brush->setLayer(0);
	if(brush->layer() > canvas->layerCount()-1)
		brush->setLayer(canvas->layerCount()-1);

	ParupaintLayer * layer = canvas->layerAt(brush->layer());
	if(layer){
		if(brush->frame() < 0) brush->setFrame(0);
		if(brush->frame() > layer->frameCount()-1){
			brush->setFrame(layer->frameCount()-1);
			qDebug() << "adjust" << brush << "to" << brush->frame() << layer->frameCount();
		}
	}
}
