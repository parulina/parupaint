#ifndef PARUPAINTKEYS_H
#define PARUPAINTKEYS_H

#include <QKeyEvent>
#include <QStringList>
#include <QHash>

struct ParupaintKey
{
	int key;
	Qt::KeyboardModifiers mod;
	ParupaintKey() : key(0), mod(Qt::NoModifier) {}
	ParupaintKey(int, Qt::KeyboardModifiers = Qt::NoModifier);
	ParupaintKey(QString);

	QKeySequence keySequence() const;
	QString toString() const;
};

typedef QHash<QString,ParupaintKey> ParupaintKeyList;
class ParupaintKeys
{
	// name, keys
	ParupaintKeyList keys;

	public:
	ParupaintKeys(const QStringList & list);
	ParupaintKeyList keyList() const;
	QStringList keyListString();

	bool setKey(const QString & name, const QString & key);
	bool setKey(const QString & inikey);

	bool key(const QString & name, ParupaintKey & key);
	bool key(const QString & name);

	QString keyString(const QString & name);
	QKeySequence keySequence(const QString & name);
	QString keyName(int k, Qt::KeyboardModifiers m);

	void saveKeys();
	void loadKeys();
};

#endif
