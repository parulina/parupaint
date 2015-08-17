#ifndef PARUPAINTAPP_H
#define PARUPAINTAPP_H

#include <QApplication>

class ParupaintServerInstance;
class ParupaintWindow;

class ParupaintApp : public QApplication {
Q_OBJECT // for gui stuff
	private:
	ParupaintServerInstance * server;
	ParupaintWindow * main_window;

	public:
	~ParupaintApp();
	ParupaintApp(int &argc, char **argv);

	protected:
	bool event(QEvent * e);

};

#endif
