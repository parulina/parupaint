#include "parupaintLayerModes.h"


QPainter::CompositionMode svgLayerModeToCompositionMode(QString mode)
{
	if(mode == "svg:src-over")	return QPainter::CompositionMode_SourceOver;
	if(mode == "svg:multiply")	return QPainter::CompositionMode_Multiply;
	if(mode == "svg:screen")	return QPainter::CompositionMode_Screen;
	if(mode == "svg:overlay")	return QPainter::CompositionMode_Overlay;
	if(mode == "svg:darken")	return QPainter::CompositionMode_Darken;
	if(mode == "svg:lighten")	return QPainter::CompositionMode_Lighten;
	if(mode == "svg:color-dodge")	return QPainter::CompositionMode_ColorDodge;
	if(mode == "svg:color-burn")	return QPainter::CompositionMode_ColorBurn;
	if(mode == "svg:hard-light")	return QPainter::CompositionMode_HardLight;
	if(mode == "svg:soft-light")	return QPainter::CompositionMode_SoftLight;
	if(mode == "svg:difference")	return QPainter::CompositionMode_Difference;
	if(mode == "svg:plus")		return QPainter::CompositionMode_Plus;
	if(mode == "svg:dst-in")	return QPainter::CompositionMode_DestinationIn;
	if(mode == "svg:dst-out")	return QPainter::CompositionMode_DestinationOut;
	if(mode == "svg:src-atop")	return QPainter::CompositionMode_SourceAtop;
	if(mode == "svg:dst-atop")	return QPainter::CompositionMode_DestinationAtop;
}

QString compositionModeToString(int mode)
{
	switch(mode){
		case QPainter::CompositionMode_SourceOver: 	return "Normal";
		case QPainter::CompositionMode_Multiply: 	return "Multiply";
		case QPainter::CompositionMode_Screen:		return "Screen";
		case QPainter::CompositionMode_Overlay:		return "Overlay";
		case QPainter::CompositionMode_Darken:		return "Darken";
		case QPainter::CompositionMode_Lighten:		return "Lighten";
		case QPainter::CompositionMode_ColorDodge:	return "Dodge";
		case QPainter::CompositionMode_ColorBurn:	return "Burn";
		case QPainter::CompositionMode_HardLight:	return "Hard light";
		case QPainter::CompositionMode_SoftLight:	return "Soft light";
		case QPainter::CompositionMode_Difference:	return "Difference";
		case QPainter::CompositionMode_Plus:		return "Plus";
		case QPainter::CompositionMode_DestinationIn:	return "Dest in";
		case QPainter::CompositionMode_DestinationOut:	return "Dest out";
		case QPainter::CompositionMode_SourceAtop:	return "Srce ontop";
		case QPainter::CompositionMode_DestinationAtop:	return "Dest ontop";
		default: return "IDK";
	}
}
