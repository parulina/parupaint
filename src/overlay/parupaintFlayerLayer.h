#ifndef PARUPAINTFLAYERLAYER_H
#define PARUPAINTFLAYERLAYER_H

#include <QWidget>

class ParupaintFlayerFrame;
class QRadioButton;
class ParupaintCustomComboBox;
class QLineEdit;

class ParupaintFlayerLayer : public QWidget
{
Q_OBJECT
	private:
	QRadioButton * layer_visible;
	ParupaintCustomComboBox * layer_mode;
	QLineEdit * layer_name;

	int layer;

	public:
	ParupaintFlayerLayer(int l, QWidget * = nullptr, bool visible = true, int mode = 0, const QString & name = "");
	void setLayerVisible(bool visible);
	void setLayerMode(int mode);
	void setLayerName(const QString & name);

	void addFrame(ParupaintFlayerFrame *);
	ParupaintFlayerFrame * frameAt(int i);
	void selectFrame(int f);

	private slots:
	void layerToggleVisibility(bool toggle);
	void layerChangeName(const QString & name);
	void layerChangeMode(int mode);

	signals:
	void visibleChanged(int l, bool visible);
	void nameChanged(int l, const QString & name);
	void modeChanged(int l, int mode);
};

#endif
