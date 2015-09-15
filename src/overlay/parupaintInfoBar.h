#ifndef PARUPAINTINFOBAR_H
#define PARUPAINTINFOBAR_H

#include <QStringList>
#include <QWidget>

class QTextBrowser;
class QLabel;

class ParupaintInfoBar : public QWidget
{
	QString current_title;
	QString current_dimensions;
	QString current_lfstatus;

	QTextBrowser * title;
	QLabel * key_list;
	
	void ReloadTitle();

	public:
	ParupaintInfoBar(QWidget * = nullptr);
	void SetCurrentTitle(QString);
	void SetCurrentDimensions(int, int);
	void SetCurrentLayerFrame(int, int);
	void SetKeyList(QStringList);
};

#endif
