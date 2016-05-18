#include "parupaintVisualCanvas.h"

#include <QDebug>
#include <QPainter>
#include <QStyleOptionGraphicsItem> // paint
#include <QTimer>

ParupaintCanvasModel::ParupaintCanvasModel(ParupaintVisualCanvas * panvas) :
	QAbstractTableModel(panvas)
{
}

int ParupaintCanvasModel::rowCount(const QModelIndex & index) const
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(panvas){
		return panvas->layerCount();
	}
	return 0;
}
int ParupaintCanvasModel::columnCount(const QModelIndex & index) const
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(panvas){
		return 3 + panvas->totalFrameCount();
	}
	return 3;
}
QVariant ParupaintCanvasModel::data(const QModelIndex & index, int role) const
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(!panvas) return QVariant();

	if(role == Qt::SizeHintRole){
		switch(index.column()){
			case 0: return QSize(20, 20);
			case 1: return QSize(80, 20);
			case 2: return QSize(100, 20);
			default: return QSize(60, 20);
		}
	}

	ParupaintLayer * layer = panvas->layerAt((this->rowCount() > 0 ? this->rowCount()-1 : 0) - index.row());
	if(!layer) return QVariant();

	ParupaintFrame * frame = nullptr;
	if(index.column() >= 3)
		frame = layer->frameAt(index.column()-3);

	if(role == Qt::EditRole){
		switch(index.column()){
			case 1: return layer->mode();
			case 2: return layer->name();
		}
	}
	if(role == Qt::CheckStateRole){
		if(index.column() == 0) {
			return (layer->visible() ? Qt::Checked : Qt::Unchecked);
		}

	} else if(role == Qt::DisplayRole){
		switch(index.column()){
			case 1: return layer->modeString();
			case 2: return layer->name();
		}
	} else if(role == Qt::ForegroundRole){
		if(index.column() >= 3) {
			if(frame && layer->isFrameExtended(frame)) return QColor("#99a9cc");
			else if(frame && !layer->isFrameExtended(frame)) return QColor("#DDD");
			else return QColor(Qt::transparent);
		}
	} else if(role == Qt::ToolTipRole){
		if(index.column() == 2){
			return layer->name();
		}
	}
	return QVariant();
}

bool ParupaintCanvasModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(!panvas) return false;

	ParupaintLayer * layer = panvas->layerAt((this->rowCount() > 0 ? this->rowCount()-1 : 0) - index.row());
	if(!layer) return false;

	if(role == Qt::CheckStateRole) {
		if(index.column() == 0) {
			bool visible = value.toBool();
			layer->setVisible(visible);
			emit onLayerVisibilityChange(panvas->layerIndex(layer), visible);
			emit dataChanged(index, index);
			return true;
		}
	}
	if(role == Qt::EditRole) {
		switch(index.column()){
			case 1: {
				int mode = value.toInt();
				layer->setMode(mode);
				emit onLayerModeChange(panvas->layerIndex(layer), mode);
				emit dataChanged(index, index);
				return true;
			}
			case 2: {
				QString name = value.toString();
				layer->setName(name);
				emit onLayerNameChange(panvas->layerIndex(layer), name);
				emit dataChanged(index, index);
				return true;
			}
		}
	}
	return false;
}

Qt::ItemFlags ParupaintCanvasModel::flags(const QModelIndex & index) const
{
	if(index.column() == 0)
		return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;
	else if(index.column() == 1)
		return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
	else if(index.column() == 2)
		return Qt::ItemIsEnabled | Qt::ItemIsEditable;
	else
		return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable;
}

void ParupaintCanvasModel::updateLayout()
{
	emit this->layoutChanged();
}

void ParupaintCanvasModel::updateLayer(int l)
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	ParupaintLayer * layer = panvas->layerAt(l);

	if(!panvas) return;
	if(layer){
		int row = panvas->layerIndex(layer) - (this->rowCount() > 0 ? this->rowCount()-1 : 0);

		emit this->dataChanged(this->index(row, 0), this->index(row, this->columnCount(this->index(row, 0))));
	}
}


ParupaintVisualCanvas::ParupaintVisualCanvas(QGraphicsItem * parent) :
	QGraphicsItem(parent),
	canvas_model(this),
	current_layer(0), current_frame(0),
	canvas_preview(true),
	checker_pixmap("#cccccc", "#ffffff"),
	flash_timeout(new QTimer(this)), fillpreview_timeout(new QTimer(this))
{
	this->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
	this->resize(QSize(500, 500));

	flash_timeout->setSingleShot(true);
	fillpreview_timeout->setSingleShot(true);

	connect(flash_timeout, &QTimer::timeout, this, &ParupaintVisualCanvas::timeoutRedraw);
	connect(fillpreview_timeout, &QTimer::timeout, this, &ParupaintVisualCanvas::timeoutRedraw);
	connect(fillpreview_timeout, &QTimer::timeout, [&](){
		this->setFillPreview();
	});

	this->connect(this, &ParupaintPanvas::onCanvasResize, this, &ParupaintVisualCanvas::newCache);
	this->connect(this, &ParupaintPanvas::onCanvasBackgroundChange, this, &ParupaintVisualCanvas::timeoutRedraw);

	this->connect(this, &ParupaintPanvas::onCanvasContentChange, &canvas_model, &ParupaintCanvasModel::updateLayout);
	this->connect(this, &ParupaintPanvas::onCanvasLayerChange, &canvas_model, &ParupaintCanvasModel::updateLayer);

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

	if(!(!this->isPreview() && flash_timeout->isActive())){
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
		if((flash_timeout->isActive() && !this->isPreview()) && (i != current_layer)) continue;

		// if layer is hidden or if debug is on, and if it's not the current layer, hide it
		if((!layer->visible()) && (i != current_layer)) continue;
		bool debug_layer = !this->isPreview();


		if(!debug_layer) painter.setCompositionMode(static_cast<QPainter::CompositionMode>(layer->mode()));
		if(layer && current_frame < layer->frameCount()){
			ParupaintFrame* frame = layer->frameAt(current_frame);
			if(frame){
				painter.setOpacity(this->isPreview() ? frame->opacity() : (debug_layer ? 1.0 : 0.2));
				painter.drawImage(area, frame->image(), area);
			}
			if(current_layer == i){
				if(fillpreview_timeout->isActive()){
					painter.drawPixmap(area, fillpreview_pixmap, area);
				}
				// only draw onionskin if preview is off.
				if(!this->isPreview()){

					int prev_frame = current_frame - 1,
					    next_frame = current_frame + 1;

					// make the onionskin opaque
					painter.setOpacity(0.2);
					if((frame = layer->frameAt(prev_frame))){
						painter.drawImage(area, frame->image(), area);
					}
					if((frame = layer->frameAt(next_frame))){
						painter.drawImage(area, frame->image(), area);
					}
				}
			}
		}
	}
	// reset
	painter.restore();

	// todo move them in the correct layer pos?
	if(!pastepreview_pixmap.isNull()){
		painter.setOpacity(1.0);
		painter.drawPixmap(QRectF(pastepreview_pos, pastepreview_pixmap.size()), 
				pastepreview_pixmap, pastepreview_pixmap.rect());
	}

	if(!line_preview.isNull()){
		QPen line_pen(line_color);
		line_pen.setWidthF(line_thickness);
		line_pen.setCapStyle(Qt::RoundCap);

		if(line_color.alpha() == 0) {
			painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);
			line_pen.setColor(Qt::white);
		}

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
void ParupaintVisualCanvas::setCurrentLayerFrame(int l, int f)
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

ParupaintCanvasModel * ParupaintVisualCanvas::model()
{
	return &canvas_model;
}
