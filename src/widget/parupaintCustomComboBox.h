#ifndef PARUPAINTCUSTOMCOMBOBOX_H
#define PARUPAINTCUSTOMCOMBOBOX_H

#include <QComboBox>

class ParupaintCustomComboBox : public QComboBox
{
Q_OBJECT
	private:
	void paintEvent(QPaintEvent * event);
	public:
	ParupaintCustomComboBox(QWidget * = nullptr);
};

#endif
