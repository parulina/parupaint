#ifndef PARUPAINTAPP_H
#define PARUPAINTAPP_H

#include <QApplication>

class ParupaintServerInstance;

class ParupaintApp : public QApplication {
Q_OBJECT // for gui stuff
	private:
	ParupaintServerInstance * server;

	public:
	ParupaintApp(int &argc, char **argv);

	protected:
	bool QtEvent(QEvent * e);

};

#endif
