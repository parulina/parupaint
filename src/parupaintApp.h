#ifndef PARUPAINTAPP_H
#define PARUPAINTAPP_H

#include <QApplication>

class ParupaintWindow;

class ParupaintApp : public QApplication
{
Q_OBJECT
	private:
	ParupaintWindow * main_window;

	protected:
	bool event(QEvent * event);

	public:
	ParupaintApp(int &argc, char **argv);

};

#endif
