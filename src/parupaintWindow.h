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
class ParupaintProjectInfo;
class ParupaintFlayer;
class ParupaintColorPicker;
class ParupaintInfoBar;
class ParupaintNetJoinPrompt;
class ParupaintNetInfo;

class ParupaintKeys;
class ParupaintPatternPopup;
class ParupaintToolPopup;

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

	QPointF 	current_pen, origin_pen;
	qreal 		origin_zoom;
	QString 	local_server;

	bool 		tablet_pen_switch;

	// important stuff
	ParupaintBrushGlass *	brushes;
	ParupaintKeys *		key_shortcuts;

	ParupaintChat *		chat;
	ParupaintProjectInfo * 	project_info;
	ParupaintFlayer * 	flayer;
	ParupaintColorPicker * 	picker;
	ParupaintInfoBar * 	infobar;

	ParupaintNetJoinPrompt*	netjoin;
	ParupaintNetInfo* 	netinfo;

	ParupaintCanvasView * 	view;
	ParupaintCanvasScene * 	scene;
	ParupaintClientInstance * client;

	ParupaintPatternPopup * pattern_popup;
	ParupaintToolPopup *	tool_popup;

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
	QString saveName() const;
	void setLocalServer(const QString &);

	signals:
	void doLocalSessionPassword(const QString &);

	private slots:
	void OnPenPress(const penInfo &);
	void OnPenMove(const penInfo &);
	void OnPenRelease(const penInfo &);
	void OnPenPointer(const penInfo &);
	void OnPenScroll(QWheelEvent *);

	void patternPopupSelect(int pattern);
	void toolPopupSelect(int tool);

	void OnNetworkConnect();
	void OnNetworkDisconnect(QString reason = "");
};

#endif
