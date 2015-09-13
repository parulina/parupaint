#ifndef PARUPAINTCANVASBANNER_H
#define PARUPAINTCANVASBANNER_H

#include <QWidget>
#include <QLabel>
#include <QTimer>

class ParupaintCanvasBanner : public QWidget
{
	private:
	QString main_text;
	QString pre_text;

	QLabel *label;
	QTimer show_timer;
	void RefreshLabel();

	public:
	ParupaintCanvasBanner(QWidget * = nullptr);
	void Show(qreal, QString = QString());
	void SetMainText(QString);
	void SetPreText(QString);

};

#endif
