#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QJsonObject>
#include <QDebug>

#include "server_bundled.h"

class ParupaintSingleServer: public QCoreApplication
{
	// bundled server includes join/leave messages
	ParupaintBundledServer * instance;
	public:
	ParupaintSingleServer(int & argc, char ** argv) : QCoreApplication(argc, argv)
	{
		this->setApplicationName("Parupaint server");

		QCommandLineParser parser;
		parser.setApplicationDescription("Stand-alone server for parupaint");
		parser.addHelpOption();
		parser.addOption({"nolog", "Disable record log"});
		parser.addOption({"password", "Server password", "password"});
		parser.process(*this);

		instance = new ParupaintBundledServer(1108, this);

		if(parser.isSet("password")){
			QString password = parser.value("password");
			qDebug() << "Server password:" << password;
			instance->setPassword(password);
		}
		if(!parser.isSet("nolog")){
			instance->startRecord();
		}
	}
};

// Single server
int main(int argc, char ** argv)
{
	ParupaintSingleServer server(argc, argv);
	return server.exec();
}
