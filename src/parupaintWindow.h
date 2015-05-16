#ifndef PARUPAINTWINDOW_H
#define PARUPAINTWINDOW_H

#include <QMainWindow>

class ParupaintCanvasPool;

class ParupaintWindow : public QMainWindow {
Q_OBJECT
	private:
	ParupaintCanvasPool * canvas;
	public:
	ParupaintWindow();
	void UpdateTitle();
};





#endif
