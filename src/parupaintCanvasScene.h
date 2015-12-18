#ifndef PARUPAINTCANVASSCENE_H
#define PARUPAINTCANVASSCENE_H

#include <QGraphicsScene>
#include <QList>

class ParupaintVisualCanvas;
class ParupaintCanvasBrush;

class ParupaintBrush; // updateMainCursor
class ParupaintVisualCursor;

class ParupaintCanvasScene : public QGraphicsScene
{
Q_OBJECT
	private:
	ParupaintVisualCanvas * panvas;

	ParupaintVisualCursor * main_cursor;
	QList<ParupaintVisualCursor*> cursors;

	public:
	~ParupaintCanvasScene();
	ParupaintCanvasScene(QObject* parent = nullptr);
	ParupaintVisualCanvas * canvas();

	void updateMainCursor(ParupaintBrush *);
	ParupaintVisualCursor * mainCursor();

	void addCursor(ParupaintVisualCursor *);
	void removeCursor(ParupaintVisualCursor *);
	void clearCursors();

	void setPreview(bool p);
	bool isPreview();
};





#endif
