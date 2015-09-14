#ifndef PARUPAINTKEYS_H
#define PARUPAINTKEYS_H

#include <QKeyEvent>
#include <QStringList>
#include <QHash>

struct ParupaintKey
{
	int key;
	Qt::KeyboardModifiers mod;
	bool shortcut;
	ParupaintKey() : key(0), mod(Qt::NoModifier), shortcut(false) {}
	ParupaintKey(int, bool = false, Qt::KeyboardModifiers = Qt::NoModifier);
	ParupaintKey(QString);

	QKeySequence GetKeySequence();
	QString GetString();
};

class ParupaintKeys
{
	// name, keys
	QHash<QString,ParupaintKey> keys;

	public:
	ParupaintKeys(QStringList);
	void AddKey(QString);
	QStringList GetKeys();

	ParupaintKey 	Get(QString);
	QKeySequence 	GetKeySequence(QString);

	int 		GetKey(QString);
	Qt::KeyboardModifiers GetModifiers(QString);

	QString Match(int, Qt::KeyboardModifiers = Qt::NoModifier);

	void Save();
	void Load();
};

#endif
