

#include "parupaintApp.h"

#include "src/net/parupaintServerInstance.h"

int main(int argc, char *argv[]){

	ParupaintApp app(argc, argv);

	ParupaintServerInstance server(1108);
	return app.exec();
}
