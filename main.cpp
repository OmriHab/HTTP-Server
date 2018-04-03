#include <iostream>
#include <sstream>
#include <map>
#include <string.h>

#include "server.h"
#include "sock_incl.h"
#include "HTTPMessage.h"
#include "HTTPServer.h"


int main(int argc, char* argv[]) {

	http::HTTPServer server(8888, 10, "/home/omri/HTTPHome");

	server.Serve();

	return 0;
}
