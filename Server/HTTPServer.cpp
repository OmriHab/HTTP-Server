#include "HTTPServer.h"
#include "../Message/HTTPMessage.h"

#include <dirent.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

using namespace http;

std::set<std::string> HTTPServer::supported_methods() {
	static std::set<std::string> supported_methods = {"GET", "HEAD"};
	return supported_methods;
};

// List of default pages to search, by order, if given a directory, add as neccesary
std::list<std::string> HTTPServer::indexes() {
	static std::list<std::string> indexes = {
		"index.html", "index.htm", "index.php"
	};
	return indexes;
}


HTTPServer::HTTPServer(int port, int max_connections, const std::string& home_path, bool verbose)
	: server::server(port, max_connections, verbose)
	, home_path(home_path)
{
	if (this->home_path.back() == '/') {
		this->home_path.pop_back();
	}
}

void HTTPServer::HandleMessage(const std::string& msg, const tcpSocket& socket) {
	StringMap http_request = HTTPMessage::ParseHTTPRequest(msg);
	StringMap URLParams;
	std::string HTTPResponse;

	try {
		URLParams    = HTTPMessage::GetURLParams(http_request["path"]);
		HTTPResponse = MakeMessage(http_request, URLParams);
	}
	catch (const std::exception& e) {
		ThreadSafeLog("Error responsing to socket " + std::to_string(socket.GetSockId()) + "\n" + e.what() + "\n", std::cerr);
	}

	socket.Send(HTTPResponse);
	// End connection after responding
	this->EndConnection(socket);
}

std::string HTTPServer::MakeMessage(const StringMap& http_request, const StringMap& URLParams) {
	try {
		std::string method = http_request.at("method");
		std::string path   = http_request.at("path");

		// If method isn't in the list of supported methods
		if (supported_methods().find(method) == supported_methods().end()) {
			return HTTPMessage::Error501();
		}

		std::string file_to_send;
		std::string full_path = this->home_path + path;
		// Ignore directory backslash
		if (full_path.back() == '/') {
			full_path.pop_back();
		}
		// Translate path, translating all special chars
 		full_path = TranslatePath(full_path);

		// If access forbiden there
		if (!LegalPath(full_path)) {
			return HTTPMessage::Error403();
		}
		// If file not found
		if (!FileExists(full_path, file_to_send)) {
			return HTTPMessage::Error404();
		}

		// If file found and in legal place
		else {
			// Read file
			std::ifstream requested_file(file_to_send);
			if (errno == EACCES) {
				// If permission denied, return a forbidden 403
				return HTTPMessage::Error403();
			}
			if (requested_file.bad()) {
				// If failed to open file, although it exists, return an error 500, internal
				return HTTPMessage::Error500();
			}

			// Read file
			std::stringstream file_contents_stream;
			file_contents_stream << requested_file.rdbuf();
			std::string file_contents = file_contents_stream.str();

			std::string extension = file_to_send.substr(file_to_send.find_last_of(".")+1);
			if (extension == "chc") {
				file_contents = HTTPMessage::CHCIt(file_contents, URLParams);
			}


			return HTTPMessage::Success200(file_contents, file_to_send);
		}

	}

	// If http_request had no "method" or "path"
	catch (const std::out_of_range& e) {
		ThreadSafeLog("HTTPServer::HandleMessage: Error, msg corrupted, aborting\n", std::cerr);
		return HTTPMessage::Error400();
	}
	// On any other exception
	catch (const std::exception& e) {
		ThreadSafeLog("HTTPServer::HandleMessage: internal error\n", std::cerr);
		return HTTPMessage::Error500();
	}
}

bool HTTPServer::LegalPath(const std::string& path) const {
	/* For path to be legal, is has to branch from home_path */
	return (path.compare(0, home_path.length(), home_path) == 0);
}

bool HTTPServer::FileExists(const std::string& path, std::string& file_found) const {
	DIR* dir_check = opendir(path.c_str());
	
	// If path is a directory
	if (dir_check != nullptr) {
		for (const std::string& index : indexes()) {
			// If file found, check if is a reqular file
			if (access((path+"/"+index).c_str(), F_OK) != -1) {
				file_found = path+"/"+index;
				return true;
			}
		}
	}

	// If is a normal file
	if (access(path.c_str(), F_OK) != -1) {
		file_found = path;
		return true;
	}

	return false;
}

std::string HTTPServer::TranslatePath(const std::string& path) {
	std::string translated;
	std::stringstream helper;

	auto c = path.begin();

	while (c != path.end()) {
		if (*c != '%') {
			translated.push_back(*c);
			c++;
		}
		// On special chars
		else {
			unsigned int hex_number;
			c++;
			// Convert next number from hex to char
			helper << std::hex << path.substr(std::distance(path.begin(), c), 2);
			helper >> hex_number;

			translated.push_back(char(hex_number));
			helper.clear();
			c += 2;
		}
	}

	return translated;
}