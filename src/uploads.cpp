#include <webserv.hpp>

string generate_upload_page(vector<string> filenames) {
	ostringstream oss;
	oss << "<!DOCTYPE html>\n";
	oss << "<html lang=\"fr\">\n";
	oss << "<head>\n";
	oss << "	<meta charset=\"UTF-8\">\n";
	oss << "	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "	<title>Uploads</title>\n";
	oss << "	<style>\n";
	oss << "		body {\n";
	oss << "			a {\n";
	oss << "				display: inline-block;\n";
	oss << "				margin-top: 1em;\n";
	oss << "				padding: 0.5em 1em;\n";
	oss << "				background-color: #3498db;\n";
	oss << "				color: white;\n";
	oss << "				text-decoration: none;\n";
	oss << "				border-radius: 4px;\n";
	oss << "				font-size: 1em;\n";
	oss << "			}\n";
	oss << "			a.download {\n";
	oss << "				background-color: #16b84e;\n";
	oss << "			}\n";
	oss << "		}\n";
	oss << "</style>\n";
	oss << "</head>\n";
	oss << "<body>\n";
	oss << "<h1>Downloads</h1>\n";
	oss << "<table>\n";
	for (vector<string>::iterator it = filenames.begin(); it != filenames.end(); it++) {	
		oss << "<tr>\n";
		oss << "	<td>📁 " + *it + "</td>\n";
		oss << "	<td><a href=\"./downloads/" + *it + "\" class=download download=\"" + *it + "\">Download</a></td>";
		oss << "</tr>\n";
	}
	oss << "</table>\n";
	oss << "<a href=\"/\">HOME</a>\n";
	oss << "</body>\n";
	oss << "</html>\n";
	return oss.str();
}

bool check_file_availability(string rq, Data &data) {
	size_t start = rq.find("downloads/") + 10;
	string filename;
	if (start != rq.npos)
		filename = rq.substr(start);
	vector<string>::iterator it;
	for (it = data._uploads.begin(); it != data._uploads.end(); it++) {
		if (filename == *it)
			break ;
	}
	if (it == data._uploads.end()) {
		return false;	
	}
	return true;
}

string find_filename(string request) {
	string body;
	string filename;
	size_t body_start;
	size_t body_end;
	string boundaryKey;
	string extension;

	body_start = request.find("\r\n\r\n");
	if (body_start == request.npos)
		throw Error("in Uploads.cpp cant find rnrn");
	body = request.substr(body_start + 4, request.length() - body_start);
	if (body.find("\n") != body.npos) {
		body_end = body.find("\n", 0);
		if (body_end == body.npos)
			throw Error("Didn't find the boundary key.");
		boundaryKey = body.substr(0, body_end - 1);
	}
	if (body.find("filename=") != body.npos) {
		body_start = body.find("filename=");
		if (body_start != body.npos)
			body_start += 10;
		else
			throw Error("Didn't find filename=");
		body_end = body.find("\"", body_start);
		if (body_end == body.npos) {
			body_end = body.find("&", body_start);
			body_start--;
			extension = ".txt";
		}
		if (body_end == body.npos) {
			body_end = body.find("\n", body_start);
			extension.clear();
		}
		if (body_end == body.npos)
			throw Error("Didn't find filename's end.");
		filename = body.substr(body_start, body_end - body_start) + extension;
	}
	return filename;
}