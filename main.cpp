#include <iostream>
#include <sstream>
#include <map>
#include <string.h>

#include "Socket/SocketIncludes.h"
#include "Message/HTTPMessage.h"
#include "Server/HTTPServer.h"


int main(int argc, char* argv[]) {
	static const std::string USAGE_MSG = std::string("Usage: ") + argv[0] + " <port> <base_path>\n";

	if (argc != 3 || std::string(argv[1]) == "-h" || std::string(argv[1]) == "--help") {
		std::cout << USAGE_MSG;
		return 1;
	}

	int port;
	try {
		port = std::stoi(argv[1]);
	}
	catch (const std::invalid_argument& e) {
		std::cout << USAGE_MSG;
		std::cout << argv[1] << " not a valid number" << std::endl;
		return 1;
	}
	catch (const std::out_of_range& e) {
		std::cout << USAGE_MSG;
		std::cout << "Please enter a port number between 0-65535" << std::endl;
		return 1;
	}

	std::string base_path = argv[2];

	http::HTTPServer server(port, 10, base_path);

	server.Serve();

	return 0;
}
