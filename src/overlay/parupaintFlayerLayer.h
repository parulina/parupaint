#ifndef PARUPAINTFLAYERLAYER_H
#define PARUPAINTFLAYERLAYER_H

#include <QWidget>

class ParupaintFlayerFrame;
class QRadioButton;
class QLineEdit;

class ParupaintFlayerLayer : public QWidget
{
Q_OBJECT
	private:
	QRadioButton * layer_visible;
	QLineEdit * layer_name;

	public:
	ParupaintFlayerLayer(QWidget * = nullptr);
	void setLayerVisible(bool hidden);
	void setLayerName(const QString & name);

	void addFrame(ParupaintFlayerFrame *);
	ParupaintFlayerFrame * frameAt(int i);

	signals:
	void visibleChanged();
};

#endif
