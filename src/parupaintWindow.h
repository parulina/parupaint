#ifndef PARUPAINTWINDOW_H
#define PARUPAINTWINDOW_H

#include <QMainWindow>

class ParupaintCanvasPool;

class ParupaintChat;
class ParupaintFlayerList;
class ParupaintColorPicker;


class ParupaintWindow : public QMainWindow {
Q_OBJECT
	private:
	QKeySequence OverlayKeyShow;
	QKeySequence OverlayKeyHide;
	bool OverlayButtonDown;


	ParupaintChat * chat;
	ParupaintFlayerList * flayer;
	ParupaintColorPicker * picker;

	void closeEvent(QCloseEvent * event);
	void resizeEvent(QResizeEvent * event);
	ParupaintCanvasPool * canvas;

	public:
	ParupaintWindow();
	void ShowOverlay(bool=true);
	void HideOverlay();


	void UpdateTitle();

	public slots:
	void OverlayKey();
};





#endif
