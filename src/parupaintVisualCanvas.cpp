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
		return panvas->totalFrameCount();
	}
	return 3;
}
QVariant ParupaintCanvasModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(!panvas) return QVariant();

	if(orientation == Qt::Vertical){
		ParupaintLayer * layer = panvas->layerAt((this->rowCount() > 0 ? this->rowCount()-1 : 0) - section);

		if(role == LayerVisibleRole) return layer->visible();
		if(role == LayerModeRole) return layer->mode();
		if(role == LayerNameRole) return layer->name();

	} else if(orientation == Qt::Horizontal){
		if(role == Qt::DisplayRole) return QVariant("Frame " + QString::number(section+1));
	}
	return QVariant();
}

QVariant ParupaintCanvasModel::data(const QModelIndex & index, int role) const
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(!panvas) return QVariant();

	if(role == Qt::SizeHintRole){
		return QSize(60, 20);
	}

	ParupaintLayer * layer = panvas->layerAt((this->rowCount() > 0 ? this->rowCount()-1 : 0) - index.row());
	if(!layer) return QVariant();

	ParupaintFrame * frame = layer->frameAt(index.column());

	if(role == Qt::BackgroundRole){
		if(frame && layer->isFrameExtended(frame)) return QBrush(QColor("#99a9cc"));
		else if(frame && !layer->isFrameExtended(frame)) return QBrush(QColor("#DDD"));
		else return QBrush(QColor(Qt::transparent));
	}
	return QVariant();
}

bool ParupaintCanvasModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant & value, int role)
{
	ParupaintPanvas * panvas = qobject_cast<ParupaintPanvas*>(this->parent());
	if(!panvas) return false;
	if(orientation == Qt::Vertical){
		ParupaintLayer * layer = panvas->layerAt(((this->rowCount() > 0 ? this->rowCount()-1 : 0)  - section));
		if(!layer) return false;

		if(role == LayerVisibleRole){
			layer->setVisible(value.toBool());
			emit onLayerVisibilityChange(panvas->layerIndex(layer), layer->visible());
			emit headerDataChanged(orientation, section, section);
		}
		if(role == LayerModeRole){
			layer->setMode(value.toInt());
			emit onLayerModeChange(panvas->layerIndex(layer), layer->mode());
			emit headerDataChanged(orientation, section, section);
		}
		if(role == LayerNameRole){
			layer->setName(value.toString());
			emit onLayerNameChange(panvas->layerIndex(layer), layer->name());
			emit headerDataChanged(orientation, section, section);
		}
	}

	return false;
}

Qt::ItemFlags ParupaintCanvasModel::flags(const QModelIndex & index) const
{
	return QAbstractTableModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
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
		int row = (this->rowCount() > 0 ? this->rowCount()-1 : 0) - panvas->layerIndex(layer);
		emit this->headerDataChanged(Qt::Vertical, row, row);
	}
}


ParupaintVisualCanvas::ParupaintVisualCanvas(QGraphicsItem * parent) :
	QGraphicsItem(parent),
	canvas_model(this),
	current_layer(0), current_frame(0),
	canvas_preview(false),
	checker_pixmap("#cccccc", "#ffffff"),
	flash_timeout(new QTimer(this)), fillpreview_timeout(new QTimer(this)),
	play_timer(new QTimer(this))
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

	this->connect(this, &ParupaintPanvas::onCanvasChange, this, &ParupaintVisualCanvas::updatePlayTimer);

	play_timer->setSingleShot(false);
	play_timer->stop();
	this->connect(play_timer, &QTimer::timeout, this, &ParupaintVisualCanvas::nextFrame);

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

void ParupaintVisualCanvas::updatePlayTimer()
{
	play_timer->setInterval(1000.0/this->frameRate());
}

void ParupaintVisualCanvas::play()
{
	if(this->totalFrameCount() == 1) return;
	play_timer->start(1000.0/this->frameRate());
}

void ParupaintVisualCanvas::stop()
{
	play_timer->stop();
}

void ParupaintVisualCanvas::togglePlay()
{
	if(this->isPlaying()) this->stop();
	else this->play();
}

bool ParupaintVisualCanvas::isPlaying()
{
	return play_timer->isActive();
}

void ParupaintVisualCanvas::nextFrame()
{
	int fc = 1;
	if(this->currentFrame() == this->totalFrameCount()-1) fc = -this->totalFrameCount();
	this->addCurrentLayerFrame(0, fc, false);
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

	if(!(!this->isPreview() && (flash_timeout->isActive() || fillpreview_timeout->isActive()))){
		painter.setOpacity(1.0);
		painter.fillRect(area, this->backgroundColor());
	}

	// normal view - draw all layer's current_frame
	// if preview, draw with less opacity
	painter.save();
	for(int i = 0; i < this->layerCount(); i++){
		ParupaintLayer* layer = this->layerAt(i);

		// hide layer only when not flashing
		if(!flash_timeout->isActive() && !layer->visible()) continue;

		// do not show other layers when... (flashing & debug)
		if((flash_timeout->isActive() && !this->isPreview()) && (i != current_layer)) continue;

		// if layer is hidden and not the current layer, hide it
		if((!layer->visible()) && (i != current_layer)) continue;
		bool debug_layer = this->flash_timeout->isActive() && !this->isPreview();
		bool temp_hide = (fillpreview_timeout->isActive());

		painter.setCompositionMode(static_cast<QPainter::CompositionMode>(layer->mode()));
		if(layer && current_frame < layer->frameCount()){
			ParupaintFrame* frame = layer->frameAt(current_frame);
			if(frame){
				painter.setOpacity((debug_layer || temp_hide) ? 1.0 : frame->opacity());
				if(!temp_hide) painter.drawImage(area, frame->image(), area);
			}
			if(current_layer == i){
				if(fillpreview_timeout->isActive()){
					painter.setOpacity(1.0);
					painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
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
	// flash when preview, only when changing layers
	// make a little white flash
	if(flash_timeout->isActive() && this->isPreview()){
		painter.setOpacity(1);
		painter.setCompositionMode(QPainter::CompositionMode_Multiply);
		painter.drawTiledPixmap(area, checker_pixmap, ppp);

		ParupaintLayer * layer = this->layerAt(current_layer);
		if(layer){
			painter.setCompositionMode(static_cast<QPainter::CompositionMode>(layer->mode()));
			ParupaintFrame * frame = layer->frameAt(current_frame);
			if(frame){
				painter.setOpacity(frame->opacity());
				painter.drawImage(area, frame->image(), area);
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
	this->setCurrentLayerFrame(l, f, false);
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
	if(b){
		this->addCurrentLayerFrame(0, 0, true);
	}
	this->canvas_preview = b;
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
