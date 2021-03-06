#ifndef PARUPAINTCANVASSCENE_H
#define PARUPAINTCANVASSCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QAbstractListModel>

class ParupaintVisualCanvas;
class ParupaintCanvasBrush;

class ParupaintBrush; // updateMainCursor
class ParupaintVisualCursor;

class ParupaintCursorList : public QAbstractListModel
{
Q_OBJECT
	QList<ParupaintVisualCursor*> cursor_list;
	public:
	ParupaintCursorList(QObject * = nullptr);

	void add(ParupaintVisualCursor * cursor);
	void remove(ParupaintVisualCursor * cursor);
	void clear();
	QList<ParupaintVisualCursor*> cursors();

	Q_SLOT void cursorUpdate();
	int rowCount(const QModelIndex & index) const;
	QVariant data(const QModelIndex & index, int role) const;
};

class ParupaintCanvasScene : public QGraphicsScene
{
Q_OBJECT
	private:
	ParupaintVisualCanvas * visual_canvas;
	ParupaintVisualCursor * main_cursor;
	ParupaintCursorList * cursors;

	signals:
	void onCursorAdded(ParupaintVisualCursor * cursor);

	public:
	~ParupaintCanvasScene();
	ParupaintCanvasScene(QObject* parent = nullptr);
	ParupaintVisualCanvas * canvas();
	ParupaintCursorList * cursorList() const;

	void updateMainCursor(ParupaintBrush *);
	ParupaintVisualCursor * mainCursor();

	void addCursor(ParupaintVisualCursor *);
	void removeCursor(ParupaintVisualCursor *);
	void clearCursors();
	void updateCursorsVisibility();
};

#endif
