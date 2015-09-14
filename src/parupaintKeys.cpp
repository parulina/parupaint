
#include "parupaintKeys.h"

#include <QDebug>


ParupaintKey::ParupaintKey(int k, bool sc, Qt::KeyboardModifiers km) : ParupaintKey()
{
	key = k;
	shortcut = sc;
	mod = km;
}

ParupaintKey::ParupaintKey(QString str) : ParupaintKey() {
	if(str.endsWith("!")){
		str.chop(1);
		shortcut = true;
	}
	if(str.contains("CTRL", Qt::CaseInsensitive)) mod |= Qt::ControlModifier;
	if(str.contains("SHIFT", Qt::CaseInsensitive)) mod |= Qt::ShiftModifier;
	if(str.contains("ALT", Qt::CaseInsensitive)) mod |= Qt::AltModifier;
	key = QKeySequence(str.section("+", -1))[0];
}

ParupaintKeys::ParupaintKeys(QStringList list) {
	foreach(auto entry, list){
		QStringList sep = entry.split(": ");
		if(sep.length() == 2 && !sep.first().isEmpty() && !sep.last().isEmpty()){
			keys.insert(sep.first(), ParupaintKey(sep.last()));
		}
	}
}
ParupaintKey ParupaintKeys::Get(QString name){
	if(keys.find(name) == keys.end()) return ParupaintKey();
	return keys.value(name);
}
QKeySequence ParupaintKeys::GetKeySequence(QString name) {
	return QKeySequence(Get(name).key + Get(name).mod);
}

int ParupaintKeys::GetKey(QString name) {
	return Get(name).key;
}

Qt::KeyboardModifiers ParupaintKeys::GetModifiers(QString name) {
	return Get(name).mod;
}

QString ParupaintKeys::Match(int k, Qt::KeyboardModifiers m)
{
	// match direct key
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		ParupaintKey key = *i;
		if(key.shortcut) continue;
		if(key.key == k && key.mod == m) return i.key();
	}
	return "";
}
