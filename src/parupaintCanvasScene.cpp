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

ParupaintCanvasScene::~ParupaintCanvasScene()
{
	delete panvas;
}

ParupaintCanvasScene::ParupaintCanvasScene(QObject * parent) : QGraphicsScene(parent)
{
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
	cursors.append(cursor); addItem(cursor);
}
void ParupaintCanvasScene::removeCursor(ParupaintVisualCursor * cursor)
{
	Q_ASSERT_X(cursor, "removeCursor", "Cursor is invalid");
	cursors.removeAll(cursor); removeItem(cursor);
}

void ParupaintCanvasScene::clearCursors()
{
	foreach(auto c, cursors) { removeItem(c); }
	qDeleteAll(cursors);
}
