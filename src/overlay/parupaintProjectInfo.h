#ifndef PARUPAINTPROJECTINFO_H
#define PARUPAINTPROJECTINFO_H

#include <QFrame>

class ParupaintLineEdit;
class QLabel;
class QDoubleSpinBox;

typedef QDoubleSpinBox ParupaintSpinBox;

class ParupaintProjectInfo : public QFrame
{
Q_OBJECT
	private:
	QLabel * line_title;

	ParupaintLineEdit * line_name;
	ParupaintSpinBox * line_fps;
	ParupaintLineEdit * line_bgc;

	public slots:
	void updateCanvasSlot();

	signals:
	void projectNameChanged(const QString & name);
	void frameRateChanged(const qreal fps);
	void backgroundColorChanged(const QColor & col);

	public:
	ParupaintProjectInfo(QWidget * = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
};

#endif
