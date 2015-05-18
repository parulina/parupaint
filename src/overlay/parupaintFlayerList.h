#ifndef PARUPAINTFLAYERLIST_H
#define PARUPAINTFLAYERLIST_H

#include <QList>
#include <QWidget>

class QVBoxLayout;
class ParupaintFlayerLayer;
class ParupaintFlayerFrame;

class ParupaintFlayerList : public QWidget
{
Q_OBJECT
	private:
	QList<ParupaintFlayerLayer*> Layers;
	QVBoxLayout * box;

	public:
	ParupaintFlayerList(QWidget * parent = nullptr);

	ParupaintFlayerLayer * NewLayer();
	ParupaintFlayerLayer * AddLayer(int);
	ParupaintFlayerLayer * AddLayer(int, ParupaintFlayerLayer *);
	void RemoveLayer(int);
	void MoveLayer(int, int);
	ParupaintFlayerLayer * GetLayer(int);

	void Clear();
	QVBoxLayout * NewBox();

	private slots:
	void FrameChecked(ParupaintFlayerFrame*);

	signals:
	void clickFrame(int, int);
};

#endif
