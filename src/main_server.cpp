#include <QCoreApplication>
#include <QJsonObject>
#include <QDebug>
#include "net/parupaintServerInstance.h"

class ParupaintSingleServer: public QCoreApplication
{
	ParupaintServerInstance * instance;
	private:
	void Message(const QString & id, const QJsonObject &obj)
	{
		//qDebug() << id << obj;
		if(id == "join") qDebug() << obj["name"] << "connected";
		else if(id == "disconnect") qDebug() << obj["name"] << "disconnected";
	};
	public:
	ParupaintSingleServer(int & argc, char ** argv) : QCoreApplication(argc, argv)
	{
		instance = new ParupaintServerInstance(1108);
		connect(instance, &ParupaintServerInstance::OnMessage, this, &ParupaintSingleServer::Message);
	}
};

// Single server
int main(int argc, char ** argv)
{
	ParupaintSingleServer server(argc, argv);
	return server.exec();
}
