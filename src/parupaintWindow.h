#ifndef PARUPAINTWINDOW_H
#define PARUPAINTWINDOW_H

#include "core/parupaintBrushGlass.h"

#include <QHash>
#include <QMainWindow>

class ParupaintCanvasView;
class ParupaintCanvasPool;
class ParupaintCanvasBrush;

class ParupaintClientInstance;

class ParupaintChat;
class ParupaintFlayer;
class ParupaintColorPicker;
class ParupaintInfoBar;

class QDropEvent;

enum OverlayStatus {
	OVERLAY_STATUS_HIDDEN,
	OVERLAY_STATUS_SHOWN_SMALL,
	OVERLAY_STATUS_SHOWN_NORMAL
};


class ParupaintWindow : public QMainWindow {
Q_OBJECT
	private:
	QKeySequence OverlayKeyShow;
	QKeySequence OverlayKeyHide;
	bool OverlayButtonDown;
	QTimer * OverlayTimer;
	QTimer * OverlayButtonTimer;
	
	
	QKeySequence CanvasKeySquash;
	int CanvasKeyNextLayer;
	int CanvasKeyPreviousLayer;
	int CanvasKeyNextFrame;
	int CanvasKeyPreviousFrame;

	int CanvasKeyReload;
	QKeySequence CanvasKeyOpen;
	int CanvasKeyNew;
	QKeySequence CanvasKeyQuicksave;
	QKeySequence CanvasKeySaveProject;
	int CanvasKeyPreview;
	QKeySequence CanvasKeyConnect;
	int CanvasKeyChat;

	QKeySequence BrushKeyUndo;
	QKeySequence BrushKeyRedo;
	QKeySequence BrushKeySwitchBrush;
	int BrushKeyPickColor;


	void UpdateOverlay();
	
	OverlayStatus	OverlayState;
	
	ParupaintBrushGlass glass;

	ParupaintChat * chat;
	ParupaintFlayer * flayer;
	ParupaintColorPicker * picker;
	ParupaintInfoBar * infobar;

	void closeEvent(QCloseEvent * event);
	void mousePressEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent * event);
	void keyReleaseEvent(QKeyEvent * event);
	bool focusNextPrevChild(bool);
	void resizeEvent(QResizeEvent * event);
	void dropEvent(QDropEvent *ev);
	void dragEnterEvent(QDragEnterEvent *ev);
	ParupaintCanvasView * view;
	ParupaintCanvasPool * pool;

	ParupaintClientInstance * client;

	public:
	~ParupaintWindow();
	ParupaintWindow();
	ParupaintCanvasPool * GetCanvasPool();

	void ShowOverlay(bool=true);
	void HideOverlay();

	void UpdateTitle();

	// Dialogs
	void Connect(QString);
	void Disconnect();
	void Open(QString);
	void SaveAs(QString);
	void New(int, int);

	private slots:
	void VersionResponse(bool, QString);

	void Command(QString, QString);

	void OverlayTimeout();
	void ButtonTimeout();

	void BrushKey();
	void NetworkKey();

	void ViewUpdate();

	void PenDrawStart(ParupaintBrush*);
	void PenMove(ParupaintBrush*);
	void PenDrawStop(ParupaintBrush*);

	void CursorChange(ParupaintBrush*);
	void ColorChange(QColor);

	void SelectFrame(int, int);
	void ChangedFrame(int, int);
	void ChatMessage(QString);
	void ChatMessageReceived(QString, QString);
};


#endif
