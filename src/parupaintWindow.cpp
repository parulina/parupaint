
#include "parupaintWindow.h"
#include "parupaintCanvasView.h"
#include "parupaintCanvasPool.h"


ParupaintWindow::ParupaintWindow() : QMainWindow()
{
	// have a ParupaintCanvas that shows canvas+mouses??
	// ui could be after parupaintcanvas?
	
	auto * canvasView = new ParupaintCanvasView(this);
	setCentralWidget(canvasView);
	
	canvas = new ParupaintCanvasPool(this);
	canvas->setBackgroundBrush(QColor(255,255,255));
	canvasView->SetCanvas(canvas);


	UpdateTitle();

	resize(500, 500);
	show();
}

void ParupaintWindow::UpdateTitle()
{
	setWindowTitle(QString("parupaint"));
}
