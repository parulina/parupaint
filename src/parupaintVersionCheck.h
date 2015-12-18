#ifndef PARUPAINTVERSIONCHECK_H
#define PARUPAINTVERSIONCHECK_H

#include <QObject>

class QNetworkReply;
class ParupaintVersionCheck : public QObject
{
Q_OBJECT
	public:
	ParupaintVersionCheck(QObject * = nullptr);

	private slots:
	void completed(QNetworkReply*);

	signals:
	void updateResponse(const QString & msg, bool update);
};

#endif
