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
#include <QStandardPaths>

#include "net/parupaintServerInstance.h"
#include "net/parupaintClientInstance.h"
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

		// log dir is by default in ./
		// when using built in server in the client, place it at %appdata%
		QDir appdata(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
		appdata.mkpath(appdata.path());

		server->setServerDir(appdata.path());
		server->startRecord();

		connect(main_window, &ParupaintWindow::doLocalSessionPassword, server, &ParupaintBundledServer::setPassword);

		QString password;
		password = cfg.value("client/sessionpassword", password).toString();
		if(password != "none"){
			if(password.isEmpty()){
				QString charas("ABCDEFGHIJKLMOPQRSTUVWXYZ0123456789");
				for(int i = 0; i < 5; i++) password += QChar(charas.at(rand() % charas.length()));
			}

			server->setPassword(password);
		}
		if(!server->password().isEmpty()){
			main_window->addChatMessage("Your session password is: <span class=\"msg-highlight\">" + server->password() + "</span>");
		}

		server_str = QString("localhost:%1").arg(server_port);
		main_window->setLocalServer(server_str);
	}
	main_window->doConnect(server_str);

	if(first_start){
		QStringList list = {
			"<span class=\"msg-highlight\">Welcome to parupaint!</span>",
			"You're currently drawing with yourself, but others are able to join you.",
			"Your current nickname is '" + main_window->networkClient()->name() + "'.<br/>Type '/name &lt;name&gt;' to set it to something else."
		};
		main_window->addChatMessage(list.join("<br/>"));
	}
}

bool ParupaintApp::event(QEvent * event)
{
	if(event->type() == QEvent::TabletEnterProximity){
		QTabletEvent * tablet_event = static_cast<QTabletEvent*>(event);
		tablet_event->accept();

		main_window->simulateCursorPositionUpdate();
		return true;
	}
	if(event->type() == QEvent::TabletLeaveProximity){
		QTabletEvent * tablet_event = static_cast<QTabletEvent*>(event);
		tablet_event->accept();

		main_window->simulateCursorPositionUpdate();
		return true;
	}
	return this->QApplication::event(event);
}
