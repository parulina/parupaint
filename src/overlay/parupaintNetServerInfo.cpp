#include "parupaintNetServerInfo.h"

// provide list of players, and a counter for spectators.

ParupaintNetServerInfo::ParupaintNetServerInfo(QWidget * parent) :
	QFrame(parent)
{
	this->resize(100, 300);
}

QSize ParupaintNetServerInfo::minimumSizeHint() const
{
	return QSize(200, 200);
}
