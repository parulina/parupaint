#include "parupaintCommonOperations.h"

// this provides a common set of functions for canvas and brushes

#include "parupaintPanvas.h"

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
			for(int i = 0; i < -lc; i++){
				canvas->removeLayer(l);
			}
			changed = true;
		} else if(lc > 0){
			for(int i = 0; i < lc; i++){
				canvas->insertLayer(l, 1);
			}
			changed = true;
		}
	}
	if(fc != 0){
		ParupaintLayer * ff = canvas->layerAt(l);
		if(ff) {
			if(fc < 0 && ff->frameCount() > 1){
				if(extend){
					for(int i = 0; i < -fc; i++)
						ff->redactFrame(f);
				} else {
					for(int i = 0; i < -fc; i++)
						ff->removeFrame(f);
				}
				changed = true;
			} else if(fc > 0){
				if(extend){
					for(int i = 0; i < fc; i++)
						ff->extendFrame(f);
				} else {
					for(int i = 0; i < fc; i++)
						ff->insertFrame(canvas->dimensions(), f);
				}
				changed = true;
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
			if(attr == "frame-opacity"){
				qreal v = val.toDouble();
				if(v > 1.0) v = 1.0;
				if(v < 0.0) v = 0.0;
				frame->setOpacity(v);
				return true;
			}
		}
	}

	return false;
}
