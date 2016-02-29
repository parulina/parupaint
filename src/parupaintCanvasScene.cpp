#include "parupaintCanvasScene.h"
// ParupaintCanvasScene handles cursors and canvas object.
// there should be one object handling the canvas,
// and each cursor should be an object themselves.
// locally controlled cursor should be an object.

#include <QDebug>
#include <QPainter>
#include <QSettings>
#include <QGraphicsView>

#include "parupaintVisualCanvas.h"
#include "parupaintVisualCursor.h"


ParupaintCursorList::ParupaintCursorList(QObject * parent) :
	QAbstractListModel(parent)
{
}
void ParupaintCursorList::add(ParupaintVisualCursor * cursor)
{
	emit this->layoutAboutToBeChanged();
	cursors.append(cursor);
	emit this->layoutChanged();

	connect(cursor, &ParupaintVisualCursor::onCursorNameChange, this, &ParupaintCursorList::cursorUpdate);
	connect(cursor, &ParupaintVisualCursor::onCursorStatusChange, this, &ParupaintCursorList::cursorUpdate);
	connect(cursor, &ParupaintBrush::onToolChange, this, &ParupaintCursorList::cursorUpdate);
	connect(cursor, &ParupaintBrush::onColorChange, this, &ParupaintCursorList::cursorUpdate);
}
void ParupaintCursorList::remove(ParupaintVisualCursor * cursor)
{
	emit this->layoutAboutToBeChanged();
	cursors.removeAll(cursor);
	emit this->layoutChanged();
}
void ParupaintCursorList::clear()
{
	qDeleteAll(cursors);

	emit this->layoutAboutToBeChanged();
	cursors.clear();
	emit this->layoutChanged();
}

void ParupaintCursorList::cursorUpdate()
{
	ParupaintVisualCursor * cursor = qobject_cast<ParupaintVisualCursor*>(sender());
	if(cursor){
		QModelIndex index = this->index(cursors.indexOf(cursor), 0);
		emit this->dataChanged(index, index);
	}
}

int ParupaintCursorList::rowCount(const QModelIndex & index) const
{
	return cursors.size();
}

QVariant ParupaintCursorList::data(const QModelIndex & index, int role) const
{
	ParupaintVisualCursor * cursor = nullptr;
	if(index.row() >= 0 && index.row() < cursors.size())
		cursor = cursors.at(index.row());

	if(role == Qt::FontRole){
		QFont font;
		if(cursor){
			if(cursor->status() == ParupaintVisualCursorStatus::CursorStatusTyping){
				font.setUnderline(true);
			}
		}
		return font;
	}
	if(role == Qt::DisplayRole){
		return cursor ? cursor->cursorName() : "n/a";
	}
	if(role == Qt::UserRole){
		if(cursor){
			return cursor->color();
		}
	}
	if(role == Qt::UserRole + 1){
		if(cursor){
			return cursor->status() ? -cursor->status() : cursor->tool();
		}
	}
	return QVariant();
}


ParupaintCanvasScene::~ParupaintCanvasScene()
{
	delete panvas;
}

ParupaintCanvasScene::ParupaintCanvasScene(QObject * parent) : QGraphicsScene(parent)
{
	cursors = new ParupaintCursorList;

	this->addItem(panvas = new ParupaintVisualCanvas());
	connect(panvas, &ParupaintVisualCanvas::onCanvasResize, [&](){
		QSettings cfg;

		const qreal pad = cfg.value("client/canvaspadding", 200).toDouble();
		this->setSceneRect(panvas->boundingRect().adjusted(-pad, -pad, pad, pad));
	});

	panvas->resize(QSize(500, 500));
	panvas->newCanvas(1, 1);

	// create cursor
	addItem((main_cursor = new ParupaintVisualCursor()));
}

ParupaintVisualCanvas * ParupaintCanvasScene::canvas()
{
	Q_ASSERT(panvas);
	return panvas;
}

ParupaintCursorList * ParupaintCanvasScene::cursorList() const
{
	return cursors;
}

void ParupaintCanvasScene::updateMainCursor(ParupaintBrush * cursor)
{
	// Catch the whole area, if the cursor gets smaller
	QRectF rect = main_cursor->sceneBoundingRect();

	cursor->copyTo(*qobject_cast<ParupaintBrush*>(main_cursor));
	main_cursor->setPos(cursor->position());

	rect |= main_cursor->sceneBoundingRect();

	// FIXME despite the rect being the correct dimensions,
	// the cursor flickers when being moved around.
	// perhaps it's moving too fast and the rendering gets interrupted?
	// setting the graphicsview to fullupdatemode, or updating it here
	// fixes the issue, but it uses cpu.
	// not sure what do.
	this->update(rect);
	/*
	foreach(QGraphicsView* v, this->views()){
		v->update(v->mapFromScene(rect).boundingRect());
	}
	*/
}

ParupaintVisualCursor * ParupaintCanvasScene::mainCursor()
{
	return main_cursor;
}

void ParupaintCanvasScene::addCursor(ParupaintVisualCursor * cursor)
{
	Q_ASSERT_X(cursor, "addCursor", "Cursor is invalid (failed to create?)");

	cursors->add(cursor);
	addItem(cursor);
}
void ParupaintCanvasScene::removeCursor(ParupaintVisualCursor * cursor)
{
	Q_ASSERT_X(cursor, "removeCursor", "Cursor is invalid");
	cursors->remove(cursor);
	removeItem(cursor);
}

void ParupaintCanvasScene::clearCursors()
{
	cursors->clear();
}
