#ifndef PARUPAINTLINEEDIT_H
#define PARUPAINTLINEEDIT_H

#include <QLineEdit>

class ParupaintLineEdit : public QLineEdit
{
Q_OBJECT
	public:
	ParupaintLineEdit(QWidget * = nullptr, const QString placeholder = "");
};

#endif
