#include "parupaintVisualCanvas.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem> // paint
#include <QTimer>

ParupaintVisualCanvas::ParupaintVisualCanvas(QGraphicsItem * parent) :
	QGraphicsItem(parent),
	current_layer(0), current_frame(0),
	canvas_preview(true),
	checker_pixmap("#cccccc", "#ffffff")
{
	this->resize(QSize(500, 500));

	flash_timeout = new QTimer(this);
	fillpreview_timeout = new QTimer(this);

	flash_timeout->setSingleShot(true);
	fillpreview_timeout->setSingleShot(true);
	connect(flash_timeout, &QTimer::timeout, this, &ParupaintVisualCanvas::timeoutRedraw);
	connect(fillpreview_timeout, &QTimer::timeout, this, &ParupaintVisualCanvas::timeoutRedraw);

	connect(fillpreview_timeout, &QTimer::timeout, [&](){
		this->setFillPreview();
	});


	this->connect(this, &ParupaintPanvas::onCanvasResize, this, &ParupaintVisualCanvas::newCache);
	this->connect(this, &ParupaintPanvas::onCanvasBackgroundChange, this, &ParupaintVisualCanvas::timeoutRedraw);

	this->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
}
void ParupaintVisualCanvas::timeoutRedraw()
{
	this->redraw();
}

QRectF ParupaintVisualCanvas::boundingRect() const
{
	return QRect(QPoint(0, 0), this->dimensions());
}

void ParupaintVisualCanvas::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *)
{
	const QRect exposed = option->exposedRect.toAlignedRect();
	painter->drawPixmap(exposed, canvas_cache, exposed);
}

inline QRect lineToRect(const QLine & line)
{
	return QRect(
		(line.x1() < line.x2() ? line.x1() : line.x2()),
		(line.y1() < line.y2() ? line.y1() : line.y2()),
		(line.x1() > line.x2() ? line.x1() : line.x2()),
		(line.y1() > line.y2() ? line.y1() : line.y2())
	);
}

void ParupaintVisualCanvas::setPreviewLine(const QLine & line, qreal width, const QColor color)
{
	QRect rect = lineToRect(line).adjusted(-width/2, -width/2, width/2, width/2);

	if(!this->line_preview.isNull()){
		rect |= lineToRect(line_preview).adjusted(-line_thickness/2, -line_thickness/2, line_thickness/2, line_thickness/2);
	}
	rect = this->matrix().mapRect(rect);
	if(rect.x() < 0) rect.adjust(-rect.x(), 0, -rect.x(), 0);
	if(rect.y() < 0) rect.adjust(0, -rect.y(), 0, -rect.y());

	this->line_preview = line;
	this->line_thickness = width;
	this->line_color = color;

	this->redraw(rect);
}

void ParupaintVisualCanvas::newCache()
{
	canvas_cache = QPixmap(this->dimensions());
	fillpreview_pixmap = QPixmap(this->dimensions());

	canvas_cache.fill(Qt::transparent);
	fillpreview_pixmap.fill(Qt::transparent);
}

void ParupaintVisualCanvas::redraw(QRect area)
{
	if(area.isNull()) area = this->boundingRect().toRect();
	QPainter painter(&canvas_cache);

	const QPointF ppp(area.x() % checker_pixmap.width(), area.y() % checker_pixmap.height());
	painter.drawTiledPixmap(area, checker_pixmap, ppp);

	// preview should not be onion skin, but then again it shows a checkerboard background...
	// a preview shows the current frame, no onionskin, with all of the layers
	// non preview shows the current frame with onionskin, with the layers in the background
	// when navigating, only the current layer should be shown...

	if(this->isPreview() && !flash_timeout->isActive()){
		painter.setOpacity(1.0);
		painter.fillRect(area, this->backgroundColor());
	}

	// normal view - draw all layer's current_frame
	// if preview, draw with less opacity
	painter.save();
	for(int i = 0; i < this->layerCount(); i++){
		ParupaintLayer* layer = this->layerAt(i);
		if(!layer->visible()) continue;

		// do not show other layers when... (flashing & preview)
		if((flash_timeout->isActive() && this->isPreview()) && (i != current_layer)) continue;

		painter.setCompositionMode(static_cast<QPainter::CompositionMode>(layer->mode()));
		if(layer && current_frame < layer->frameCount()){
			ParupaintFrame* frame = layer->frameAt(current_frame);
			if(frame){
				painter.setOpacity(this->isPreview() ? frame->opacity() : 0.6);
				painter.drawImage(area, frame->image(), area);
			}
			// only draw onionskin if preview is off.
			if((!this->isPreview()) && current_layer == i){

				int prev_frame = current_frame - 1,
				    next_frame = current_frame + 1;

				// make the onionskin opaque
				painter.setOpacity(0.3);
				if((frame = layer->frameAt(prev_frame))){
					painter.drawImage(area, frame->image(), area);
				}
				if((frame = layer->frameAt(next_frame))){
					painter.drawImage(area, frame->image(), area);
				}
			}
		}
	}
	// reset
	painter.restore();

	// now draw the focused frame
	if(!this->isPreview() || flash_timeout->isActive()){
		// always draw current frame with max op
		ParupaintLayer * layer = this->layerAt(current_layer);
		if(layer != nullptr){

			ParupaintFrame* frame = layer->frameAt(current_frame);
			if(frame != nullptr) {
				painter.setOpacity(this->isPreview() ? frame->opacity() : 1.0);
				painter.drawImage(area, frame->image(), area);
			}
		}
	}

	if(fillpreview_timeout->isActive()){
		painter.drawPixmap(area, fillpreview_pixmap, area);
	}
	if(!pastepreview_pixmap.isNull()){
		painter.setOpacity(1.0);
		painter.drawPixmap(QRectF(pastepreview_pos, pastepreview_pixmap.size()), 
				pastepreview_pixmap, pastepreview_pixmap.rect());
	}

	if(!line_preview.isNull()){
		QPen line_pen(line_color);
		line_pen.setWidthF(line_thickness);
		line_pen.setCapStyle(Qt::RoundCap);
		painter.setPen(line_pen);
		painter.setOpacity(1.0);
		painter.drawLine(line_preview);
	}
	this->update(area);
}

void ParupaintVisualCanvas::setPastePreview(const QImage & image, const QPointF & pos)
{
	if(image.isNull()){
		QRect rect(pastepreview_pos.toPoint(), pastepreview_pixmap.size());
		pastepreview_pixmap = QPixmap();
		this->redraw(rect);
		return;
	}
	pastepreview_pixmap = QPixmap::fromImage(image);
	this->setPastePreviewPosition(pos);
}
void ParupaintVisualCanvas::setPastePreviewPosition(const QPointF & pos)
{
	QRect area(pastepreview_pos.toPoint(), pastepreview_pixmap.size());
	pastepreview_pos = pos;
	area |= QRect(pastepreview_pos.toPoint(), pastepreview_pixmap.size());
	this->redraw(area.adjusted(-1, -1, 1, 1));
}
bool ParupaintVisualCanvas::hasPastePreview()
{
	return !(pastepreview_pixmap.isNull());
}
void ParupaintVisualCanvas::setFillPreview(const QImage & image)
{
	if(image.isNull()){
		fillpreview_pixmap = QPixmap();
		return;
	}
	fillpreview_pixmap = QPixmap::fromImage(image);
	fillpreview_timeout->start(500);
	this->redraw();
}
void ParupaintVisualCanvas::current_lf_update(int l, int f)
{
	this->setCurrentLayerFrame(l, f, true);
}
void ParupaintVisualCanvas::setCurrentLayerFrame(int l, int f, bool flash)
{
	if(flash) flash_timeout->start(700);

	current_layer = l; current_frame = f;

	if(current_layer < 0) current_layer = 0;
	if(current_layer && current_layer >= this->layerCount()) current_layer = this->layerCount() - 1;

	ParupaintLayer * layer = this->layerAt(current_layer);
	if(layer){
		if(current_frame < 0) current_frame = 0;
		if(current_frame && current_frame >= layer->frameCount()) current_frame = layer->frameCount()-1;
	}

	this->redraw();
	emit onCurrentLayerFrameChange(current_layer, current_frame);
}
void ParupaintVisualCanvas::addCurrentLayerFrame(int lc, int fc, bool flash)
{
	this->setCurrentLayerFrame(this->currentLayer() + lc, this->currentFrame() + fc, flash);
}

void ParupaintVisualCanvas::adjustCurrentLayerFrame(bool flash)
{
	this->setCurrentLayerFrame(this->currentLayer(), this->currentFrame(), flash);
}

int ParupaintVisualCanvas::currentLayer() const
{
	return current_layer;
}
int ParupaintVisualCanvas::currentFrame() const
{
	return current_frame;
}
ParupaintLayer * ParupaintVisualCanvas::currentCanvasLayer()
{
	return this->layerAt(this->currentLayer());
}
ParupaintFrame * ParupaintVisualCanvas::currentCanvasFrame()
{
	ParupaintLayer * layer = this->currentCanvasLayer();
	if(!layer) return nullptr;
	return layer->frameAt(this->currentFrame());
}

void ParupaintVisualCanvas::setPreview(bool b)
{
	this->canvas_preview = b;
	this->redraw();
}
bool ParupaintVisualCanvas::isPreview()
{
	return this->canvas_preview;
}

const QPixmap & ParupaintVisualCanvas::canvasCache() const
{
	return canvas_cache;
}
