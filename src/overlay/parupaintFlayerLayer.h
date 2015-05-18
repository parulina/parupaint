#ifndef PARUPAINTFLAYERLAYER_H
#define PARUPAINTFLAYERLAYER_H

#include <QWidget>
#include <QList>

class ParupaintFlayerFrame;
class QHBoxLayout;

class ParupaintFlayerLayer : public QWidget
{
Q_OBJECT
	private:
	QHBoxLayout * box;
	QList<ParupaintFlayerFrame*> Frames;

	public:
	int  Index;
	ParupaintFlayerLayer(QWidget * parent = nullptr);
	

	ParupaintFlayerFrame * NewFrame();
	ParupaintFlayerFrame * AddFrame(int);
	ParupaintFlayerFrame * AddFrame(int, ParupaintFlayerFrame *);
	void RemoveFrame(int);
	void MoveFrame(int, int);
	ParupaintFlayerFrame * GetFrame(int);
	void Clear();
	void ClearChecked();

	private slots:
	void FrameChecked(bool);

	signals:
	void frameCheck(ParupaintFlayerFrame*);
};


#endif
