
#include "parupaintKeys.h"

#include <QSettings>
#include <QDebug>


ParupaintKey::ParupaintKey(int k, Qt::KeyboardModifiers km) : ParupaintKey()
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
void ParupaintKeys::AddKey(QString entry) {
	QStringList sep = entry.simplified().split("=");
	if(sep.length() == 2 && !sep.first().isEmpty() && !sep.last().isEmpty()){
		keys.insert(sep.first(), ParupaintKey(sep.last()));
	} else if(!sep.first().isEmpty() && sep.last().isEmpty()){
		keys.remove(sep.first());
	}
}
QStringList ParupaintKeys::GetKeys() {
	QStringList list;
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		ParupaintKey key = *i;
		QString name = i.key();
		list << (name + "=" + key.GetString());
	}
	return list;
}

QKeySequence ParupaintKey::GetKeySequence() {
	return QKeySequence(key + mod);
}

QString ParupaintKey::GetString() {
	return GetKeySequence().toString();
}


ParupaintKeys::ParupaintKeys(QStringList list) {
	foreach(auto entry, list){
		this->AddKey(entry);
	}
}
ParupaintKey ParupaintKeys::Get(QString name){
	if(keys.find(name) == keys.end()) return ParupaintKey();
	return keys.value(name);
}
QKeySequence ParupaintKeys::GetKeySequence(QString name) {
	return Get(name).GetKeySequence();
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
		if(key.key == k && key.mod == m) return i.key();
	}
	return "";
}

void ParupaintKeys::Save() {
	QSettings cfg;
	cfg.beginGroup("keys");
	for(auto i = keys.constBegin(); i != keys.constEnd(); ++i){
		ParupaintKey key = *i;
		QString name = i.key();
		cfg.setValue(name, key.GetString());
	}
}

void ParupaintKeys::Load() {
	QSettings cfg;
	cfg.beginGroup("keys");
	foreach(const QString & key, cfg.childKeys()){
		if(key.startsWith("toolswitch_")) continue;
		QString val = key + "=" + cfg.value(key).toString();
		this->AddKey(val);
	}
}
