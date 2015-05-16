#ifndef PARUPAINTAPP_H
#define PARUPAINTAPP_H

#include <QApplication>

class ParupaintApp : public QApplication {
Q_OBJECT // for gui stuff
	public:
	ParupaintApp(int &argc, char **argv);

	protected:
	bool QtEvent(QEvent * e);

};

#endif
