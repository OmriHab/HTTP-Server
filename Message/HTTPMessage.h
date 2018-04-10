#ifndef HTTP_MESSAGE_H
#define HTTP_MESSAGE_H

#include <map>

namespace http {


class HTTPMessage {
private:
	typedef std::map<std::string, std::string> StringMap;
	// Delete c'tor, d'tor and equals operator, class HTTPMessage is a static class
	HTTPMessage() = delete;
	~HTTPMessage() = delete;
	HTTPMessage& operator=(const HTTPMessage& other) = delete;

	// Get date in the format used by http requests
	static std::string GetDate();

public:
	static StringMap ParseHTTPRequest(const std::string& request);
	static StringMap GetURLParams(const std::string& URL);
	
	static std::string BuildHTTPResponse(const std::string& HTTPVersion, const std::string& Code, const std::string& phrase, const StringMap& params, const std::string& body);
	static std::string BuildErrorBody(int ErrorCode, const std::string& ErrorMessage);
	
	static std::string GetContentType(const std::string& file_name);

	/* CHC functions, like php but in Cpp! */
	/**
	* Converts a CHC file into a an html file.
	* chc_body - CHC file text to make into an html file.
	* Return value: Normal html file after CHC'ing it.
	*/
	static std::string CHCIt(const std::string& chc_body, const StringMap& URLParams);

private:
	static std::string CHCToCPP(const std::string& chc_code);
	/**
	* Runs the CHC code and returns the output.
	* chc_code - code to run.
	* Return value: Output of given chc code.
	*/
	static std::string RunIt(const std::string& chc_code);

public:

	/**
	* Success and error calls, separated for ease of use
	* Codes -
	* Success Codes: 
	* 	200: Success, sending back what was asked for
	*		 Called with a body string as the answer
	* Client Error Codes:
	* 	400: Bad Request
	*		 The request could not be understood by the server due to malformed syntax
	*	403: Forbiden
	*		 The server understood the request, but is refusing to fulfill it
	*	404: Not Found
	*		 The server has not found anything matching the Request-URI
	* Server Error Codes:
	*	500: Internal Server Error
	*		 The server encountered an unexpected condition which prevented it from fulfilling the request
	*	501: Not Implemented
	*		 The server does not support the functionality required to fulfill the request
	*/

	static std::string Success200(const std::string& body, const std::string& file_name);
	static std::string Error400();
	static std::string Error403();
	static std::string Error404();
	static std::string Error500();
	static std::string Error501();


};

}

#endif