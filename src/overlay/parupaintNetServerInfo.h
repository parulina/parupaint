#ifndef PARUPAINTNETSERVERINFO_H
#define PARUPAINTNETSERVERINFO_H

#include <QFrame>

class ParupaintNetServerInfo : public QFrame
{
Q_OBJECT
	public:
	ParupaintNetServerInfo(QWidget * = nullptr);

	protected:
	QSize minimumSizeHint() const;
};

#endif
