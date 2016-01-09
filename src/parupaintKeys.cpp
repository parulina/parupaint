#include "parupaintKeys.h"

#include <QSettings>
#include <QDebug>

ParupaintKey::ParupaintKey(int k, Qt::KeyboardModifiers km) :
	ParupaintKey()
{
	key = k;
	mod = km;
}

ParupaintKey::ParupaintKey(QString str) : ParupaintKey() {
	if(str.contains("CTRL", Qt::CaseInsensitive)) mod |= Qt::ControlModifier;
	if(str.contains("SHIFT", Qt::CaseInsensitive)) mod |= Qt::ShiftModifier;
	if(str.contains("ALT", Qt::CaseInsensitive)) mod |= Qt::AltModifier;
	key = QKeySequence(str.section("+", -1))[0];
}

QKeySequence ParupaintKey::keySequence() const {
	return QKeySequence(key + mod);
}

QString ParupaintKey::toString() const {
	return keySequence().toString();
}

// ParupaintKeys
ParupaintKeys::ParupaintKeys(const QStringList & list)
{
	foreach(const QString & entry, list){
		this->setKey(entry);
	}
}
ParupaintKeyList ParupaintKeys::keyList() const
{
	return this->keys;
}
QStringList ParupaintKeys::keyListString()
{
	QStringList list;
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		list << (i.key() + "=" + (*i).toString());
	}
	return list;
}


bool ParupaintKeys::setKey(const QString & name, const QString & key)
{
	if(name.isEmpty()) return false;
	if(key.isEmpty()){
		// given key is empty, so just remove the key
		keys.remove(name);
	} else {
		keys.insert(name, ParupaintKey(key));
	}
	return true;
}

bool ParupaintKeys::setKey(const QString & iniKey)
{
	if(iniKey.count('=') != 1) return false;
	const QStringList sep = iniKey.simplified().split('=');

	return setKey(sep.first(), sep.last());
}

bool ParupaintKeys::key(const QString & name)
{
	return !(keys.find(name) == keys.end());
}
bool ParupaintKeys::key(const QString & name, ParupaintKey & key)
{
	if(keys.find(name) == keys.end()) return false;
	key = keys.value(name);
	return true;
}

QString ParupaintKeys::keyString(const QString & name)
{
	ParupaintKey key;
	this->key(name, key);
	return key.toString();
}

QKeySequence ParupaintKeys::keySequence(const QString & name)
{
	ParupaintKey key;
	this->key(name, key);
	return key.keySequence();
}

QString ParupaintKeys::keyName(int k, Qt::KeyboardModifiers m)
{
	QString bk;
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		ParupaintKey key = *i;
		if(key.key == k && key.mod == Qt::NoModifier) bk = i.key();
		if(key.key == k && key.mod == m) return i.key();
	}
	return bk;
}

void ParupaintKeys::saveKeys()
{
	QSettings cfg;
	cfg.beginGroup("keys");
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		ParupaintKey key = *i;
		QString name = i.key();
		cfg.setValue(name, key.toString());
	}
}

void ParupaintKeys::loadKeys()
{
	QSettings cfg;
	cfg.beginGroup("keys");
	foreach(const QString & key, cfg.childKeys()){
		// skip loading this key if it isn't already in the list
		if(!this->key(key)) continue;

		this->setKey(key, cfg.value(key).toString());
	}
}
