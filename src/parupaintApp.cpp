// (c) paru 2015
//

#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSettings>
#include <QFile>
#include <QUrl>

#include "src/net/parupaintServerInstance.h"

#include "parupaintWindow.h"
#include "parupaintApp.h"

#include "parupaintVersion.h"

// have a signal that updates the title?
// have window in this file?

#include <QDebug>

ParupaintApp::~ParupaintApp()
{
	if(server) delete server;
}

ParupaintApp::ParupaintApp(int &argc, char **argv) : QApplication(argc, argv), server(nullptr)
{
	setOrganizationName("paru");
	setOrganizationDomain("sqnya.se");
	setApplicationName("parupaint");
	setApplicationVersion(PARUPAINT_VERSION);
	setWindowIcon(QIcon(":/resources/parupaint.ico"));

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

	
	QCommandLineParser parser;
	parser.setApplicationDescription("Draw with other people and stuff");
	parser.addHelpOption();
	parser.addOption(QCommandLineOption({"c","connect"}, "Connect to a server", "address"));
	parser.addOption(QCommandLineOption({"p","port"}, "Specify port to run the server", "port"));
	parser.process(*this);

	QString server_str = parser.value("connect");
	int port_num = parser.value("port").toInt();
	if(port_num <= 0){
		port_num = 1108;
	}


	auto * win = new ParupaintWindow;
	if(server_str.isEmpty()){
		server = new ParupaintServerInstance(port_num);
		win->Connect(QString("ws://localhost"));
	} else {

		qDebug() << "Connecting to" << server_str;
		win->Connect(server_str);
	}

	QFile file(":resources/stylesheet.qss");
	if(file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		this->setStyleSheet(file.readAll());
		file.close();
	}
}
