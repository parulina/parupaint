#ifndef PARUPAINTLAYERMODES_H
#define PARUPAINTLAYERMODES_H

#include <QPainter>

const QStringList svgLayerModes = 
{
	"svg:src-over", "svg:multiply", "svg:screen", "svg:overlay", "svg:darken",
	"svg:lighten", "svg:color-dodge", "svg:color-burn", "svg:hard-light", "svg:soft-light",
	"svg:difference", "svg:plus", "svg:dst-in", "svg:dst-out", "svg:src-atop", "svg:dst-atop"
};

QPainter::CompositionMode svgLayerModeToCompositionMode(QString mode);
QString compositionModeToString(int mode);

#endif
