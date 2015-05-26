
#include "parupaintOverlayWidget.h"

ParupaintOverlayWidget::ParupaintOverlayWidget(QWidget * parent) : QWidget(parent)
{
	this->setFocusPolicy(Qt::ClickFocus);
	this->setStyleSheet("background-color:rgba(0, 0, 0, 0.5);");
}

