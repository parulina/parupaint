#ifndef PARUPAINTINFOBAR_H
#define PARUPAINTINFOBAR_H

#include <QStringList>
#include <QWidget>
#include <QTextBrowser>
#include <QLabel>

class ParupaintInfoBarText : public QTextBrowser
{
Q_OBJECT
	public:
	ParupaintInfoBarText(QWidget * = nullptr);
	QSize minimumSizeHint() const;
};

class ParupaintInfoBarKeys : public QLabel
{
Q_OBJECT
	public:
	ParupaintInfoBarKeys(QWidget * = nullptr);
	QSize minimumSizeHint() const;
};
class ParupaintInfoBarStatus : public QTextBrowser
{
Q_OBJECT
	private:
	QString connected_to;
	QString dimensions;
	QString layerframe;

	public:
	ParupaintInfoBarStatus(QWidget * = nullptr);
	void setConnectedTo(const QString &);
	void setDimensions(const QSize &);
	void setLayerFrame(int layer, int frame);
	void updateTitle();
};

class ParupaintInfoBar : public QWidget
{
Q_OBJECT
	QString current_title;
	QString current_dimensions;
	QString current_lfstatus;

	ParupaintInfoBarText * info_text;
	ParupaintInfoBarStatus * info_status;
	ParupaintInfoBarKeys * info_keys;
	
	void ReloadTitle();

	public:
	ParupaintInfoBar(QWidget * = nullptr);

	ParupaintInfoBarStatus * status();
	void setKeyList(const QStringList &);

	signals:
	void onStatusClick(const QUrl &);
};

#endif
