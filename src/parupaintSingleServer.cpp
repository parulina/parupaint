#include <QCoreApplication>
#include "net/parupaintServerInstance.h"

class ParupaintSingleServer: public QCoreApplication
{
	public:
	ParupaintSingleServer(int & argc, char ** argv) : QCoreApplication(argc, argv)
	{
		ParupaintServerInstance instance(1108);
	}
};

// Single server
int main(int argc, char ** argv)
{
	ParupaintSingleServer server(argc, argv);
	return server.exec();
}
