#ifndef PARUPAINTNETINFO_H
#define PARUPAINTNETINFO_H

#include <QStyledItemDelegate>
#include <QListView>

class ParupaintLabelIcon;

class ParupaintPlayerListPainter : public QStyledItemDelegate
{
	public:
	ParupaintPlayerListPainter(QObject * = nullptr);
	void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const;
	QSize sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const;
};

class ParupaintPlayerList : public QListView
{
Q_OBJECT
	public:
	ParupaintPlayerList(QWidget * = nullptr);
	protected:
	QSize minimumSizeHint() const;
};

class ParupaintNetInfo : public QFrame
{
Q_OBJECT
	ParupaintLabelIcon * num_painters;
	ParupaintLabelIcon * num_spectators;

	ParupaintPlayerList * list;

	public:
	ParupaintNetInfo(QWidget * = nullptr);
	void setCursorListModel(QAbstractListModel * model);

	void setNumPainters(int num);
	void setNumSpectators(int num);

	protected:
	QSize minimumSizeHint() const;
};

#endif
