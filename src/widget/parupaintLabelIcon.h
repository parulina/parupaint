#ifndef PARUPAINTLABELICON_H
#define PARUPAINTLABELICON_H

#include <QFrame>

class QLabel;

class ParupaintLabelIcon : public QFrame
{
Q_OBJECT
	QLabel * label_icon;
	QLabel * label_text;

	public:
	ParupaintLabelIcon(const QPixmap & icon, const QString & label = QString(), QWidget * = nullptr);

	void setIcon(const QPixmap & icon);
	void setText(const QString & text);

	const QPixmap & icon() const;
	const QLabel & label() const;
};


#endif
