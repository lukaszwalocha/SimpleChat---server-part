#include "pch.h"
#include "Server.h"


int main() {

	try {
		Server server;
		server.startMainEventLoop();
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
	}

	return 0;
}