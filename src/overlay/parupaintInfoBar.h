#ifndef PARUPAINTINFOBAR_H
#define PARUPAINTINFOBAR_H

#include <QStringList>
#include <QTextBrowser>
#include <QLabel>
#include <QTabWidget>

class ParupaintInfoBarText : public QTextBrowser
{
Q_OBJECT
	public:
	ParupaintInfoBarText(QWidget * = nullptr);

	protected:
	QSize minimumSizeHint() const;
};

class ParupaintInfoBarTabWidget : public QTabWidget
{
Q_OBJECT
	public:
	ParupaintInfoBarTabWidget(QWidget * = nullptr);

	protected:
	QSize minimumSizeHint() const;
	void showEvent(QShowEvent*);

	public slots:
	void currentChange();
};

class ParupaintInfoBarStatus : public QFrame
{
Q_OBJECT
	private:
	QLabel * connected_text;

	public:
	ParupaintInfoBarStatus(QWidget * = nullptr);
	void setConnectedText(const QString & text);

	protected:
	QSize minimumSizeHint() const;
};

class ParupaintInfoBar : public QFrame
{
Q_OBJECT
	ParupaintInfoBarTabWidget * tab_widget;
	ParupaintInfoBarText * info_server;
	ParupaintInfoBarText * info_text;
	ParupaintInfoBarText * info_tutorial;
	ParupaintInfoBarStatus * info_status;
	
	public:
	ParupaintInfoBar(QWidget * = nullptr);
	void setConnectedText(const QString & text);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif
