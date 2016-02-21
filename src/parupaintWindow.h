#ifndef PARUPAINTWINDOW_H
#define PARUPAINTWINDOW_H

#include "parupaintCanvasView.h" // penInfo

#include <QTabletEvent>
#include <QMainWindow>
#include <QDir>

class ParupaintCanvasView;
class ParupaintCanvasScene;

class ParupaintClientInstance;

class ParupaintBrushGlass;

class ParupaintChat;
class ParupaintFlayer;
class ParupaintColorPicker;
class ParupaintInfoBar;
class ParupaintNetJoinPrompt;
class ParupaintNetPlayerList;

class ParupaintKeys;

class QDropEvent;


enum overlayStates {
	overlayHiddenState = 0,
	overlayNormalState,
	overlayExpandedState
};

enum canvasStates {
	noCanvasState = 0,
	canvasDrawingState,
	canvasMovingState,
	canvasZoomingState,
	canvasBrushZoomingState
};


class ParupaintWindow : public QMainWindow {
Q_OBJECT
	private:
	// variables
	overlayStates	overlay_state;
	canvasStates 	canvas_state;

	bool 		overlay_button;
	QTimer * 	overlay_timeout;
	QTimer * 	overlay_button_timeout;

	QPointF 	view_pos;
	QPointF 	current_pen, origin_pen;
	qreal 		origin_zoom;
	QString 	local_server;

	bool 		tablet_pen_switch;

	// important stuff
	ParupaintBrushGlass *	brushes;
	ParupaintKeys *		key_shortcuts;

	ParupaintChat *		chat;
	ParupaintFlayer * 	flayer;
	ParupaintColorPicker * 	picker;
	ParupaintInfoBar * 	infobar;

	ParupaintNetJoinPrompt*	netjoin;
	ParupaintNetPlayerList* netlist;

	ParupaintCanvasView * 	view;
	ParupaintCanvasScene * 	scene;
	ParupaintClientInstance * client;

	void closeEvent(QCloseEvent * event);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent * event);
	void resizeEvent(QResizeEvent * event);
	void dropEvent(QDropEvent *ev);
	void dragEnterEvent(QDragEnterEvent *ev);

	public:
	ParupaintWindow(QWidget * = nullptr);
	ParupaintClientInstance * networkClient();

	void setCanvasState(canvasStates state);

	void updateOverlay();
	void showOverlay(overlayStates state);
	void hideOverlay();

	// actions
	void doQuickSave();
	void doOpen(const QString & file);
	QString doSaveAs(const QString & file);
	void doNew(int w, int h, bool resize = false);
	void doChat(const QString & msg);
	void doCommand(const QString & cmd, const QString & params);
	void doSessionPassword(const QString & sessionpw);
	void doUserName(const QString & username);
	void doConnect(const QString &);
	void doDisconnect();
	void doJoin();
	void doJoinPassword(const QString & password);

	void showOpenDialog();
	void showSaveAsDialog();
	void showNewDialog();
	void showConnectDialog();
	void showSettingsDialog();
	void showKeyBindDialog();
	void showPasswordDialog();

	void addChatMessage(const QString & msg, const QString & usr = QString());

	QSize canvasDimensions();
	QDir saveDir() const;
	void setLocalServer(const QString &);

	signals:
	void doLocalSessionPassword(const QString &);

	private slots:
	void OnPenPress(const penInfo &);
	void OnPenMove(const penInfo &);
	void OnPenRelease(const penInfo &);
	void OnPenPointer(const penInfo &);
	void OnPenScroll(QWheelEvent *);

	void OnNetworkConnect();
	void OnNetworkDisconnect(QString reason = "");
};

#endif
