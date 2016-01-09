#include "parupaintKeyBindsDialog.h"

#include <QDebug>
#include <QFormLayout>

#include "../parupaintKeys.h"

ParupaintKeyBindButton::ParupaintKeyBindButton(const QString & label, QWidget * parent) :
	QPushButton(label, parent)
{
	this->setFlat(true);
	this->setCheckable(true);
	this->setFocusPolicy(Qt::NoFocus);
}

QSize ParupaintKeyBindButton::minimumSizeHint() const
{
	return QSize(10, 10);
}

ParupaintKeyBindsDialog::ParupaintKeyBindsDialog(ParupaintKeys *keys, QWidget * parent) :
	ParupaintDialog(parent, "set keys..."),
	keys_pointer(keys), focused_keybutton(nullptr)
{
	if(!keys) return;

	QPushButton * ok_button = new QPushButton("ok", this);
	ok_button->setAutoDefault(false);
	connect(ok_button, &QPushButton::pressed, this, &QDialog::close);


	QFormLayout * form_layout = new QFormLayout;
	form_layout->setSizeConstraint(QLayout::SetFixedSize);

		const ParupaintKeyList list = keys->keyList();

		QStringList keylist = list.keys();
		keylist.sort();

		foreach(const QString & k, keylist){
			const ParupaintKey & key = list[k];

			ParupaintKeyBindButton * keybutton = new ParupaintKeyBindButton(key.toString());
			keybutton->setProperty("label", k);

			connect(keybutton, &ParupaintKeyBindButton::pressed,
				this, &ParupaintKeyBindsDialog::focusKeyButton);

			form_layout->addRow(k, keybutton);
		}
		form_layout->addRow(ok_button);

	this->setLayout(form_layout);
}

void ParupaintKeyBindsDialog::keyPressEvent(QKeyEvent * event)
{
	if(event->key() != Qt::Key_Shift &&
	   event->key() != Qt::Key_Control &&
	   event->key() != Qt::Key_Alt &&
	   event->key() != Qt::Key_AltGr){
		if(focused_keybutton && keys_pointer){

			const QString label = focused_keybutton->property("label").toString();
			const QString key = ParupaintKey(event->key(), event->modifiers()).toString();

			qDebug() << label << "=" << key;

			if(keys_pointer->setKey(label, key)){
				focused_keybutton->setText(key);
			}
			keys_pointer->saveKeys();

			focused_keybutton->setChecked(false);
			focused_keybutton = nullptr;

			event->accept();
		}
	}
	ParupaintDialog::keyPressEvent(event);
}

void ParupaintKeyBindsDialog::focusKeyButton()
{
	ParupaintKeyBindButton * button = qobject_cast<ParupaintKeyBindButton*>(sender());
	if(!button) return;

	if(this->focused_keybutton){
		focused_keybutton->setChecked(false);
	}

	this->focused_keybutton = button;
}
