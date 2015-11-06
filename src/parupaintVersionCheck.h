#ifndef PARUPAINTVERSIONCHECK_H
#define PARUPAINTVERSIONCHECK_H

#include <QObject>

class QNetworkReply;
class ParupaintVersionCheck : public QObject
{
Q_OBJECT
	public:
	ParupaintVersionCheck();

	private slots:
	void completed(QNetworkReply*);

	signals:
	void Response(bool, const QString & = "");
};

#endif
