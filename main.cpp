#include <iostream>
#include <sstream>
#include <map>
#include <string.h>

#include "Socket/SocketIncludes.h"
#include "Message/HTTPMessage.h"
#include "Server/HTTPServer.h"


int main(int argc, char* argv[]) {

	http::HTTPServer server(8888, 10, "/home/omri/HTTPHome");

	server.Serve();

	return 0;
}
