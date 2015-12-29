#include <QCoreApplication>
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
		instance = new ParupaintBundledServer(1108, this);
	}
};

// Single server
int main(int argc, char ** argv)
{
	ParupaintSingleServer server(argc, argv);
	return server.exec();
}
