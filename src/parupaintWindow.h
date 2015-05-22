#ifndef PARUPAINTWINDOW_H
#define PARUPAINTWINDOW_H

#include "core/parupaintBrush.h"

#include <QHash>
#include <QMainWindow>

class ParupaintCanvasPool;
class ParupaintCanvasBrush;

class ParupaintClientInstance;

class ParupaintChat;
class ParupaintFlayer;
class ParupaintColorPicker;
class ParupaintInfoBar;

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
	QTimer * OverlayButtonTimer;
	QTimer * OverlayTimer;
	
	
	QKeySequence CanvasKeySquash;
	QKeySequence CanvasKeyNextLayer;
	QKeySequence CanvasKeyPreviousLayer;
	QKeySequence CanvasKeyNextFrame;
	QKeySequence CanvasKeyPreviousFrame;
	QKeySequence CanvasKeyReload;


	QKeySequence BrushKeyUndo;
	QKeySequence BrushKeyRedo;


	void UpdateOverlay();
	
	OverlayStatus	OverlayState;
	
	ParupaintBrush 	brush;

	ParupaintChat * chat;
	ParupaintFlayer * flayer;
	ParupaintColorPicker * picker;
	ParupaintInfoBar * infobar;

	void closeEvent(QCloseEvent * event);
	void keyPressEvent(QKeyEvent * event);
	void resizeEvent(QResizeEvent * event);
	ParupaintCanvasPool * pool;

	ParupaintClientInstance * client;

	public:
	ParupaintWindow();
	ParupaintCanvasPool * GetCanvasPool();

	void ShowOverlay(bool=true);
	void HideOverlay();

	void UpdateTitle();

	private slots:
	void TabTimeout();
	void OverlayTimeout();

	void UndoRedoKey();
	void OverlayKey();
	void CanvasChangeKey();
	void NetworkKey();

	void ViewUpdate();

	void PenDrawStart(ParupaintBrush*);
	void PenMove(ParupaintBrush*);
	void PenDrawStop(ParupaintBrush*);

	void SelectFrame(int, int);
	void ChangedFrame(int, int);
};


#endif
