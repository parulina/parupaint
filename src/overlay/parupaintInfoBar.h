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
};

class ParupaintInfoBarTutorial : public QTextBrowser
{
Q_OBJECT
	public:
	ParupaintInfoBarTutorial(QWidget * = nullptr);
};

class ParupaintInfoBarTabWidget : public QTabWidget
{
Q_OBJECT
	public:
	ParupaintInfoBarTabWidget(QWidget * = nullptr);

	protected:
	QSize minimumSizeHint() const;
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
	ParupaintInfoBarText * info_text;
	ParupaintInfoBarText * info_tutorial;
	ParupaintInfoBarStatus * info_status;
	
	public:
	ParupaintInfoBar(QWidget * = nullptr);
	void setConnectedText(const QString & text);
};

#endif
