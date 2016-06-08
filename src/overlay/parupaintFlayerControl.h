#ifndef PARUPAINTFLAYERCONTROL_H
#define PARUPAINTFLAYERCONTROL_H

#include <QFrame>

class QRadioButton;
class QLineEdit;
class QComboBox;

class ParupaintFlayerControl : public QFrame
{
Q_OBJECT
	private:
	QRadioButton * layer_visible;
	QComboBox * layer_mode;
	QLineEdit * layer_name;

	public:
	ParupaintFlayerControl(QWidget * = nullptr);

	void setLayerVisible(bool visible);
	void setLayerMode(int mode);
	void setLayerName(const QString & name);

	signals:
	void onLayerVisibilityChange(bool visiblity);
	void onLayerModeChange(int mode);
	void onLayerNameChange(const QString & name);
};
#endif
