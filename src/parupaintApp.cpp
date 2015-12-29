#include "parupaintApp.h"

#include <QDebug>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QSettings>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QUrl>
#include <QTime>
#include <QInputDialog>

#include "net/parupaintServerInstance.h"
#include "parupaintWindow.h"
#include "parupaintVersion.h"
#include "parupaintVersionCheck.h"

#include "main/server_bundled.h"

ParupaintApp::ParupaintApp(int &argc, char **argv) : QApplication(argc, argv)
{
	setOrganizationName("paru");
	setOrganizationDomain("sqnya.se");
	setApplicationName("parupaint");
	setApplicationVersion(PARUPAINT_VERSION);
	setWindowIcon(QIcon(":/resources/parupaint.ico"));

	QSettings::setDefaultFormat(QSettings::IniFormat);

	QFontDatabase db;
	db.addApplicationFont(":/resources/parufont.ttf");

	this->setFont(QFont("parufont", 12));
	this->setStyleSheet(":/resources/stylesheet.qss");


	QFile file(":resources/stylesheet.qss");
	if(file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		this->setStyleSheet(file.readAll());
		file.close();
	}

	bool first_start(false);
	QSettings cfg;
	if(!cfg.contains("client/username") ||
	    cfg.value("client/username").toString().isEmpty()) {
#ifdef Q_OS_WIN
		QString uname = getenv("USERNAME");
#else
		QString uname = getenv("USER");
#endif
		cfg.setValue("client/username", uname);
		first_start = true;
	}

	QCommandLineParser parser;
	parser.setApplicationDescription("Draw with other people and stuff");
	parser.addHelpOption();
	parser.addOption({{"c","connect"}, "Connect to a server", "address"});
	parser.addOption({{"p","port"}, "Specify port to run the server", "port"});
	parser.addOption({QString("no-update-check"), "Don't check for updates"});
	parser.addVersionOption();
	parser.process(*this);

	QString server_str;
	int 	server_port(1108);
	bool	update_check = !parser.isSet("no-update-check");

	if(parser.isSet("connect")) server_str = parser.value("connect");
	if(parser.isSet("port")) server_port = parser.value("port").toInt();

	// feels wrong creating without a parent...
	main_window = new ParupaintWindow;
	main_window->setWindowTitle("parupaint " + QString(PARUPAINT_VERSION));

	if(update_check){
		ParupaintVersionCheck * version_check = new ParupaintVersionCheck(this);
		connect(version_check, &ParupaintVersionCheck::updateResponse, [this](const QString & msg, bool update){
			if(update) main_window->addChatMessage(msg);
		});
	}

	if(server_str.isEmpty()){
		ParupaintBundledServer* server = new ParupaintBundledServer(server_port, this);
		server->setProtective(true);

		QString password;
		password = cfg.value("client/sessionpassword", password).toString();
		if(password != "none"){
			if(password.isEmpty()){
				QString charas("ABCDEFGHIJKLMOPQRSTUVWXYZ0123456789");
				for(int i = 0; i < 5; i++) password += QChar(charas.at(rand() % charas.length()));
			}

			server->setPassword(password);
			if(!server->password().isEmpty()){
				main_window->addChatMessage("Your session password is: <span class=\"msg-highlight\">" + server->password() + "</span>");
			}
		}

		server_str = QString("localhost:%1").arg(server_port);
		main_window->setLocalServer(server_str);
	}
	main_window->doConnect(server_str);

	if(first_start){
		main_window->showSettingsDialog();
	}
}
