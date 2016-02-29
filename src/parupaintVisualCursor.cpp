#include "parupaintVisualCursor.h"
// ParupaintVisualCursor is the actual cursor shown

#include <QTimer>
#include <QGraphicsScene>
#include <QCursor>
#include <QPainter>
#include <QTransform>
#include <QPalette>
#include <QPen>
#include <QtMath>
#include <QDebug>

#include "core/parupaintBrush.h"

const qreal crosshair_size = 10;

// Name

ParupaintStaticCursorName::ParupaintStaticCursorName(QGraphicsItem * parent) : QGraphicsTextItem(parent)
{
	this->setObjectName("CursorName");
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	this->setDefaultTextColor(Qt::white);
}
void ParupaintStaticCursorName::setText(const QString & text)
{
	this->show();
	this->setPlainText(text);

	if(text.isEmpty()) this->hide();
	//zoom fucks with this again...
	//this->setX(-this->boundingRect().width()/2);
}
void ParupaintStaticCursorName::setBackgroundColor(const QColor & color)
{
	this->background_color = color;
}
void ParupaintStaticCursorName::paint(QPainter* painter, const QStyleOptionGraphicsItem* so, QWidget* widget)
{
	painter->fillRect(this->boundingRect(), background_color);

	painter->save();
		painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
		this->QGraphicsTextItem::paint(painter, so, widget);
	painter->restore();
}

// Tool icon

ParupaintStaticCursorIcon::ParupaintStaticCursorIcon(QGraphicsItem * parent) : QGraphicsPixmapItem(parent)
{
	this->setFlag(QGraphicsItem::ItemIgnoresTransformations);
}
void ParupaintStaticCursorIcon::setIconRowColumn(int row, int column, QRgb col)
{
	QImage icons(":/resources/icons.png");
	if(icons.isNull()) return;

	// set the red color to the custom one
	QVector<QRgb> rgb_list = icons.colorTable();
	rgb_list[1] = col;
	icons.setColorTable(rgb_list);

	QSize icon_size(32, 32);

	// cut it out and set as pixmap
	QRect area(QPoint(icon_size.width() * column, icon_size.height() * row), icon_size);
	this->setPixmap(QPixmap::fromImage(icons.copy(area)));
	this->setOffset(QPoint(0, -this->pixmap().height()) + QPoint(5, -5));
}



// Cursor

ParupaintVisualCursor::ParupaintVisualCursor(QGraphicsItem * parent) :
	QGraphicsItem(parent),
	current_name(""), current_status(0)
{
	this->setObjectName("Cursor");
	this->setAcceptHoverEvents(true);

	name_obj = new ParupaintStaticCursorName(this);
	tool_obj = new ParupaintStaticCursorIcon(this);
	status_obj = new ParupaintStaticCursorIcon(this);
	status_obj->setMatrix(QMatrix(-1, 0, 0, 1, 0, 0));

	status_timeout = new QTimer(this);
	status_timeout->setSingleShot(true);
	this->connect(status_timeout, &QTimer::timeout, [this](){
		this->setStatus();
	});

	// hide it as default
	name_obj->setText("");

	connect(this, &ParupaintBrush::onColorChange, this, &ParupaintVisualCursor::updateChanges);
	connect(this, &ParupaintBrush::onToolChange, this, &ParupaintVisualCursor::updateChanges);
	// needed because setPos is overloaded.
	connect(this, &ParupaintBrush::onPositionChange, [this](const QPointF & pos){
		this->setPos(pos);
	});
}

QRectF ParupaintVisualCursor::boundingRect() const
{
	qreal min_w = crosshair_size, min_h = crosshair_size;
	if(this->size() > min_w) min_w = this->size();
	if(this->size() > min_h) min_h = this->size();

	return QRectF(-min_w/2.0, -min_h/2.0, min_w, min_h).adjusted(-1, -1, 1, 1);
}

void ParupaintVisualCursor::setCursorName(const QString & name)
{
	Q_ASSERT(name_obj);
	current_name = name;

	name_obj->setText(current_name);
	emit onCursorNameChange(current_name);
}

void ParupaintVisualCursor::setStatus(int s, int timeout)
{
	if(s == current_status) return;
	current_status = s;

	status_obj->setIconRowColumn(1, current_status, this->rgba());
	emit onCursorStatusChange(current_status);

	status_timeout->stop();
	if(timeout > 0) status_timeout->start(timeout);
}

void ParupaintVisualCursor::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
	qreal p = this->pressure(),
	      w = this->size();
	if(this->tool() == ParupaintBrushToolTypes::BrushToolFloodFill)
		w = 1;

	QRectF cp((-QPointF((w*p)/2, (w*p)/2)),	QSizeF(w*p, w*p)),
	       cc((-QPointF(w/2, w/2)),		QSizeF(w, w));

	// should we draw pressure ring?
	if(w > 1 && p > 0 && p < 1){
		painter->save();

		QPen inner_pen(Qt::white);
		inner_pen.setCosmetic(true);

		painter->setPen(inner_pen);
		painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
		painter->drawEllipse(cp);

		painter->restore();
	}
	// draw outer ring
	if(true){
		painter->save();

		QPen outer_pen(Qt::white);
		outer_pen.setWidth(1);
		outer_pen.setCosmetic(true);
		if(this->color().alpha() == 0) outer_pen.setStyle(Qt::DashLine);

		painter->setPen(outer_pen);
		painter->setCompositionMode(QPainter::CompositionMode_Exclusion);

		if(w > 1){
			painter->drawEllipse(cc);

		} else if(w <= 1) {
			// Draw some crosshair
			if(painter->transform().m22() < 8){
				painter->drawLine(QLine(0, -crosshair_size/2, 0, -2));
				painter->drawLine(QLine(0, crosshair_size/2, 0, 2));
				painter->drawLine(QLine(-crosshair_size/2, 0, -2, 0));
				painter->drawLine(QLine(crosshair_size/2, 0, 2, 0));
			} else {
				painter->drawEllipse(QRectF(QPointF(-0.2, -0.2), QSizeF(.4, .4)));
				outer_pen.setWidth(2);
				painter->setPen(outer_pen);
			}
			QRectF rect(this->pixelPosition() - this->QGraphicsItem::pos(), QSizeF(1, 1));
			painter->drawRect(rect);
		}
		painter->restore();
	}
}

void ParupaintVisualCursor::hoverEnterEvent(QGraphicsSceneHoverEvent*)
{
	if(!name_obj->toPlainText().isEmpty()){
		name_obj->setOpacity(0.2);
		tool_obj->setOpacity(0.2);
	}
}
void ParupaintVisualCursor::hoverLeaveEvent(QGraphicsSceneHoverEvent*)
{
	name_obj->setOpacity(1);
	tool_obj->setOpacity(1);
}

void ParupaintVisualCursor::setSize(qreal size)
{
	this->prepareGeometryChange();
	ParupaintBrush::setSize(size);
}

int ParupaintVisualCursor::status() const
{
	return current_status;
}
QString ParupaintVisualCursor::cursorName() const
{
	return current_name;
}

void ParupaintVisualCursor::updateChanges()
{
	tool_obj->setIconRowColumn(0, this->tool(), this->rgba());
	name_obj->setBackgroundColor(this->color());

	this->update();
}
