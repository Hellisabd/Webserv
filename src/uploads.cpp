#include <webserv.hpp>

std::string uploadFile(std::string request)
{
	std::string body;
	std::string filename;
	std::string type;
	std::size_t body_start;
	std::size_t body_end;
	std::string content;
	std::string boundaryKey;

	body_start = request.find("\r\n\r\n");
	if (body_start == request.npos)
		throw Error("in Uploads.cpp cant find rnrn");
	body = request.substr(body_start + 4, request.length() - body_start);
	if (body.find("\n") != body.npos)
	{
		body_end = body.find("\n", 0);
		if (body_end == body.npos)
			throw Error("Didn't find the boundary key.");
		boundaryKey = body.substr(0, body_end - 1);
	}
	if (body.find("filename=") != body.npos)
	{
		body_start = body.find("filename=");
		if (body_start != body.npos)
			body_start += 10;
		else
			throw Error("Didn't find filename=");
		body_end = body.find("\"", body_start);
		if (body_end == body.npos)
			throw Error("Didn't find last quote");
		filename = body.substr(body_start, body_end - body_start);
	}
	if (body.find("Content-Type: ") != body.npos)
	{
		body_start = body.find("Content-Type: ");
		if (body_start != body.npos)
			body_start += 14;
		body_end = body.find("\n", body_start);
		if (body_end == body.npos)
			throw Error("Didn't find new line char");
		type = body.substr(body_start, body_end - body_start - 1);
		if (type != "application/octet-stream" && type != "text/plain")
			return ("415");
			// throw Error("Wrong type of file for upload.");
			// mettre en place page 415 plutot que throw une erreur
			// segfault qund on load un PNG par ex, requete fragmenteee
	}
	if (body.find(boundaryKey, body_end) != body.npos)
	{
		body_start = body_end + 3;
		body_end = body.find(boundaryKey, body_start);
		body = body.substr(body_start, body_end - body_start - 1);
	}
	filename = "./site/downloads/" + filename;
	std::ofstream file(filename.c_str());
	if (file.is_open())
	{
		file << body;
		file.close();
	}
	return filename;
}

void generate_uploads_url(std::vector<string> filenames)
{
	std::ostringstream oss;
	oss << "<!DOCTYPE html>\n";
	oss << "<html lang=\"fr\">\n";
	oss << "<head>\n";
	oss << "	<meta charset=\"UTF-8\">\n";
	oss << "	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	oss << "	<title>Comments about Webserv</title>\n";
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
	oss << "			a1 {\n";
	oss << "				display: inline-block;\n";
	oss << "				margin-top: 1em;\n";
	oss << "				padding: 0.5em 1em;\n";
	oss << "				background-color: #16b84e;\n";
	oss << "				color: white;\n";
	oss << "				text-decoration: none;\n";
	oss << "				border-radius: 4px;\n";
	oss << "				font-size: 1em;\n";
	oss << "			}\n";
	oss << "		}\n";
	oss << "</style>\n";
	oss << "</head>\n";
	oss << "<body>\n";
	oss << "<h1>Upload</h1>\n";
	oss << "	<form action=\"/upload\" method=\"POST\" enctype=\"multipart/form-data\">\n";
	oss << "		<label for=\"file\">Pick a file:</label>\n";
	oss << "		<input type=\"file\" id=\"file\" name=\"file\">\n";
	oss << "		<button type=\"submit\">Upload</button>\n";
	oss << "	</form>\n";
	oss << "<h2>Downloads</h2>\n";
	oss << "<table>\n";
	for (std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); it++) {	
	oss << "<tr>";
	oss << "<td>📁 " + *it + "</td>";
	oss << "<td><a1 href=\"./downloads/" + *it + "\" download=\"" + *it + "\">Download</a1></td>";
	oss << "</tr>\n";
	}
	oss << "</table>\n";
	oss << "<a href=\"/\">HOME</a>\n";
	oss << "</body>\n";
	oss << "</html>\n";
	std::ofstream page("./site/upload.html");
	if (page.is_open()) {
		page << oss.str();
		page.close();
	}
}