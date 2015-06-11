#ifndef PARUPAINTINFOBAR_H
#define PARUPAINTINFOBAR_H

#include <QWidget>

class QTextBrowser;

class ParupaintInfoBar : public QWidget
{
	QString current_title;
	QString current_dimensions;
	QString current_lfstatus;

	QTextBrowser * title;
	
	void ReloadTitle();

	public:
	ParupaintInfoBar(QWidget * = nullptr);
	void SetCurrentTitle(QString);
	void SetCurrentDimensions(int, int);
	void SetCurrentLayerFrame(int, int);
};

#endif
