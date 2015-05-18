// (c) paru 2015
//

#include <QSettings>
#include <QFile>

#include "parupaintWindow.h"
#include "parupaintApp.h"

// have a signal that updates the title?
// have window in this file?

ParupaintApp::ParupaintApp(int &argc, char **argv) : QApplication(argc, argv)
{
	setOrganizationName("paru");
	setOrganizationDomain("sqnya.se");
	setApplicationName("parupaint");

#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
	setApplicationDisplayName("parupaint");
#endif

	// Set default username
	QSettings cfg;
	cfg.beginGroup("painter");
	if(!cfg.contains("username") || 
		cfg.value("username").toString().isEmpty()) {
		
#ifdef Q_OS_WIN
		QString uname = getenv("USERNAME");
#else
		QString uname = getenv("USER");
#endif
		cfg.setValue("username", uname);
	}
	cfg.endGroup();

	

	auto * win = new ParupaintWindow;
	QFile file(":resources/stylesheet.qss");
	if(file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		this->setStyleSheet(file.readAll());
		file.close();
	}

}
