#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <set>

#include "server.h"

namespace http {

class HTTPServer : public server {
private:
	static std::set<std::string> supported_methods();
	static std::list<std::string> indexes();

	std::string home_path;

public:
	HTTPServer(int port, int max_connections, const std::string& home_path, bool verbose = true);
	~HTTPServer() = default;
	
	virtual void HandleMessage(const std::string& msg, const tcpSocket& socket);
	
private:
	std::string MakeMessage(const std::map<std::string, std::string>& http_request);

	bool LegalPath(const std::string& path) const;
	inline bool FileExists(const std::string& path, std::string& file_to_send) const;
	
	static std::string TranslatePath(const std::string& path);

};

}

#endif