#ifndef PARUPAINTVISUALCURSOR_H
#define PARUPAINTVISUALCURSOR_H

#include "core/parupaintBrush.h"

#include <QGraphicsObject>
#include <QGraphicsTextItem>
#include <QPixmap> // cached icon

class QTimer;

enum ParupaintVisualCursorStatus {
	CursorStatusNone = 0,
	CursorStatusTyping,

	CursorStatusMax // don't move!
};

class ParupaintStaticCursorName : public QGraphicsTextItem
{
	private:
	QColor background_color;

	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);

	public:
	ParupaintStaticCursorName(QGraphicsItem* = nullptr);
	void setText(const QString & text);
	void setBackgroundColor(const QColor &);
};
class ParupaintStaticCursorIcon : public QGraphicsPixmapItem
{
	public:
	ParupaintStaticCursorIcon(QGraphicsItem* = nullptr);
	void setIconRowColumn(int row, int column, QRgb col);
};

class ParupaintVisualCursor : public ParupaintBrush, public QGraphicsItem
{
Q_OBJECT
Q_INTERFACES(QGraphicsItem)
	private:
	ParupaintStaticCursorName * name_obj;
	ParupaintStaticCursorIcon * tool_obj;
	ParupaintStaticCursorIcon * status_obj;

	QTimer * status_timeout;

	QString current_name;
	int current_status;

	void paint(QPainter*, const QStyleOptionGraphicsItem*, QWidget*);
	void hoverEnterEvent(QGraphicsSceneHoverEvent*);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent*);

	signals:
	void onCursorNameChange(const QString & cursorName);
	void onCursorStatusChange(int status);

	private slots:
	void updateChanges();

	public:
	ParupaintVisualCursor(QGraphicsItem * parent = nullptr);

	QRectF boundingRect() const;

	void setCursorName(const QString & name);
	void setStatus(int s = 0, int timeout = 0);

	int status() const;
	QString cursorName() const;

	// reimplemented
	void setSize(qreal size);
};

#endif
