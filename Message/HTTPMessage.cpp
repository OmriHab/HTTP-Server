#include "HTTPMessage.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>


using namespace http;

std::map<std::string, std::string> HTTPMessage::ParseHTTPRequest(const std::string& request) {
	std::map<std::string, std::string> HTTPRequest;

	std::string::const_iterator head = request.begin();
	std::string::const_iterator tail = head;
	std::string::const_iterator colon;
	std::string key;
	std::string val;

	try {
		// Parse first line
		tail = std::find(head, request.end(), ' ');
		HTTPRequest["method"] = std::string(head, tail);
		head = tail + 1;

		tail = std::find(head, request.end(), ' ');
		HTTPRequest["path"] = std::string(head, tail);
		head = tail + 1;

		tail = std::find(head, request.end(), '\r');
		HTTPRequest["version"] = std::string(head, tail);
		head = tail + 2;

		// Parse the rest
		while (request.substr(std::distance(request.begin(), head), 2) != "\r\n") {
			tail = std::find(head, request.end(), '\r');

			colon = std::find(head, tail, ':');

			key = std::string(head, colon);
			val = std::string(colon+2, tail);

			HTTPRequest[key] = val;

			head = tail + 2;
		}
	}

	catch (const std::exception& e) {
		throw std::runtime_error("Error: request broken");
	}
	return HTTPRequest;
}

std::string HTTPMessage::BuildHTTPResponse(const std::string& HTTPVersion, const std::string& Code, const std::string& phrase, const std::map<std::string, std::string>& params, const std::string& body) {
	std::string response;
	// First line
	response = HTTPVersion + " " + Code +  " " + phrase + "\r\n";
	// Header parameters
	for (const auto& param : params) {
		response += param.first + ": " + param.second + "\r\n";
	}

	response += "\r\n";

	response += body;

	return response;
}

std::string HTTPMessage::BuildErrorBody(int ErrorCode, const std::string& ErrorMessage) {
	std::string body;

	body += "<html>";
	body +=   "<head><title>Error " + std::to_string(ErrorCode) + "</title></head>";
	body +=   "<body>";
	body +=     "<h1>Error " + std::to_string(ErrorCode) + ": "+ ErrorMessage + "</h1>";
	body +=     "<p>We apologize for the inconvenience</p>";
	body +=   "</body>";
	body += "</html>";

	return body;
}

std::string HTTPMessage::CHCIt(const std::string& chc_body) {
	// Find the chc tag
	int chc_tag = chc_body.find("<?chc");
	int chc_end = -2;

	std::string final_result;

	// Itterate over all chc sections
	while (chc_tag != std::string::npos) {
		std::string chc_section;

		// Add from end of last chc section to beggining of current chc section
		final_result += chc_body.substr(chc_end+2, chc_tag-(chc_end+2));

		// Find the closing tag, starting from the opening tab
		chc_end = chc_body.find("?>", chc_tag);
		if (chc_end == std::string::npos) {
			// On bad chc section, set length as (body length)-2 so
			// the chc section doesnt get copied to final_result
			chc_end = chc_body.length()-2;
			break;
		}

		// Set chc section as code between the tags
		chc_section = chc_body.substr(chc_tag+5, chc_end-2-(chc_tag+5));
		try {
			chc_section = CHCToCPP(chc_section);
			// Add to final result the output of the chc section
			final_result += HTTPMessage::RunIt(chc_section);
		}
		catch (const std::exception& e) {
			; // On exception, leave an empty space
		}
		
		chc_tag = chc_body.find("<?chc", chc_end);
	}

	final_result += chc_body.substr(chc_end+2);

	return final_result;
}

std::string HTTPMessage::CHCToCPP(const std::string& chc_code) {
	/**
	* CHC code is like cpp code, just it doesnt need to have
	* the main function in it, for simplicity of writing.
	* This function wraps the CHC code in a main function.
	*/
	std::string cpp_string;
	bool before_hash = true;
	std::string::const_iterator code_itter;

	for (code_itter = chc_code.begin(); code_itter != chc_code.end(); code_itter++) {
		if (*code_itter == '#') {
			before_hash = false;
		}
		else if (before_hash && !std::isspace(*code_itter)) {
			break;
		}
		if (*code_itter == '\n') {
			before_hash = true;
		}
		cpp_string.push_back(*code_itter);
	}

	return cpp_string + "int main(){ " + std::string(code_itter, chc_code.end()) + " }";


}

std::string HTTPMessage::RunIt(const std::string& chc_code) {
	static const std::string CPP_FILE_PATH = "/tmp/chc_run.cpp";
	static const std::string EXE_FILE_PATH = "/tmp/chc_run";
	std::ofstream cpp_file;
	
	cpp_file.open(CPP_FILE_PATH, std::ofstream::out | std::ofstream::trunc);
	if (cpp_file.bad()) {
		throw std::runtime_error("HTTPMessage::RunIt(): Error opening tmp file");
	}
	cpp_file << chc_code;
	cpp_file.close();

	/* Compile cpp file */
	if (system(("g++ " + CPP_FILE_PATH + " -o " + EXE_FILE_PATH + " 1>&2 2>/dev/null").c_str()) != 0) {
		throw std::runtime_error("HTTPMessage::RunIt(): Error compiling chc code");
	}

	/* Run compiled cpp file and return output */
	std::array<char, 128> buffer;
    std::string result;
    std::shared_ptr<FILE> program_output(popen(EXE_FILE_PATH.c_str(), "r"), pclose);

    if (program_output == nullptr) {
		throw std::runtime_error("HTTPMessage::RunIt(): Error running code");
    }
    /* Get output from program */
    while (!feof(program_output.get())) {
        if (fgets(buffer.data(), 128, program_output.get()) != nullptr) {
            result += buffer.data();
        }
        // If fgets fails, return what was gained until now
        else {
        	return result;
        }
    }

    // Remove both files
    remove(EXE_FILE_PATH.c_str());
    remove(CPP_FILE_PATH.c_str());

    return result;
}

std::string HTTPMessage::Success200(const std::string& body, const std::string& file_name) {
	std::string response;
	std::map<std::string, std::string> parameters;

	parameters["Date"]                = GetDate();
	parameters["Content-Length"]      = std::to_string(body.length());
	parameters["Content-Type"]        = GetContentType(file_name);


	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "200", "OK", parameters, body);
	return response;
}

std::string HTTPMessage::Error400() {
	std::string response;
	std::map<std::string, std::string> parameters;
	
	std::string body = HTTPMessage::BuildErrorBody(400, "Bad Synthax");

	parameters["Date"] = GetDate();
	parameters["Content-Length"] = std::to_string(body.length());
	parameters["Content-Type"] = "text/html";

	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "400", "Bad Request", parameters, body);
	return response;
}

std::string HTTPMessage::Error403() {
	std::string response;
	std::map<std::string, std::string> parameters;
	
	std::string body = HTTPMessage::BuildErrorBody(403, "Forbidden");

	parameters["Date"] = GetDate();
	parameters["Content-Length"] = std::to_string(body.length());
	parameters["Content-Type"] = "text/html";

	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "403", "Forbidden", parameters, body);
	return response;
}

std::string HTTPMessage::Error404() {
	std::string response;
	std::map<std::string, std::string> parameters;
	
	std::string body = HTTPMessage::BuildErrorBody(404, "Not Found");

	parameters["Date"] = GetDate();
	parameters["Content-Length"] = std::to_string(body.length());
	parameters["Content-Type"] = "text/html";

	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "404", "Not Found", parameters, body);
	return response;
}

std::string HTTPMessage::Error500() {
	std::string response;
	std::map<std::string, std::string> parameters;
	
	std::string body = HTTPMessage::BuildErrorBody(500, "Internal Server Error");

	parameters["Date"] = GetDate();
	parameters["Content-Length"] = std::to_string(body.length());
	parameters["Content-Type"] = "text/html";

	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "500", "Internal Server Error", parameters, body);
	return response;
}

std::string HTTPMessage::Error501() {

	std::string response;
	std::map<std::string, std::string> parameters;
	
	std::string body = HTTPMessage::BuildErrorBody(501, "Not Implemented");

	parameters["Date"] = GetDate();
	parameters["Content-Length"] = std::to_string(body.length());
	parameters["Content-Type"] = "text/html";

	response = HTTPMessage::BuildHTTPResponse("HTTP/1.1", "501", "Not Implemented", parameters, body);
	return response;
}

std::string HTTPMessage::GetDate() {
	char time_buffer[100] = {0};
	time_t now = time(0);
	struct tm gmtm = *gmtime(&now);
	strftime(time_buffer, sizeof(time_buffer), "%a, %d %b %Y %H:%M:%S %Z", &gmtm);

	return std::string(time_buffer);
}

std::string HTTPMessage::GetContentType(const std::string& file_name) {
	static const std::map<std::string, std::string> content_types = {
		{"html", "text/html"}, {"htm", "text/html"},       {"css", "text/css"},
		{"jpg", "image/jpeg"}, {"jpeg", "image/jpeg"},     {"png", "image/png"},
		{"gif", "image/gif"},  {"mp4", "video/mp4"},       {"mpeg", "video/mpeg"},
		{"mp3", "audio/mpeg"}, {"avi", "video/avi"},       {"mov", "video/quicktime"},
		{"wav", "audio/wav"},  {"pdf", "application/pdf"}, {"chc", "text/html"}
	};

	// Get file extension
	std::string extension = file_name.substr(file_name.find_last_of(".")+1);

	if (content_types.find(extension) != content_types.end()) {
		return content_types.at(extension);
	}

	// Default type
	return "text/plain";
}