#include <webserv.hpp>
#include <dirent.h>

void replace(std::string &com) {
	std::size_t	pos = 0;
	while (pos < com.length())
	{
		if (pos == com.find("+", pos))
			com.replace(pos, 1, " ");
		if (pos == com.find("%0D%0A", pos))
			com.replace(pos, 6, "\n");
		if (pos == com.find("%09", pos))
			com.replace(pos, 3, "\t");
		if (pos == com.find("%3A", pos))
			com.replace(pos, 3, ":");
		if (pos == com.find("%21", pos))
			com.replace(pos, 3, "!");
		if (pos == com.find("%22", pos))
			com.replace(pos, 3, "\"");
		if (pos == com.find("%23", pos))
			com.replace(pos, 3, "#");
		if (pos == com.find("%24", pos))
			com.replace(pos, 3, "$");
		if (pos == com.find("%25", pos))
			com.replace(pos, 3, "%");
		if (pos == com.find("%26", pos))
			com.replace(pos, 3, "&");
		if (pos == com.find("%27", pos))
			com.replace(pos, 3, "'");
		if (pos == com.find("%28", pos))
			com.replace(pos, 3, "(");
		if (pos == com.find("%29", pos))
			com.replace(pos, 3, ")");
		if (pos == com.find("%2B", pos))
			com.replace(pos, 3, "+");
		if (pos == com.find("%2C", pos))
			com.replace(pos, 3, ",");
		if (pos == com.find("%2F", pos))
			com.replace(pos, 3, "/");
		if (pos == com.find("%3B", pos))
			com.replace(pos, 3, ";");
		if (pos == com.find("%3C", pos))
			com.replace(pos, 3, "<");
		if (pos == com.find("%3D", pos))
			com.replace(pos, 3, "=");
		if (pos == com.find("%3E", pos))
			com.replace(pos, 3, ">");
		if (pos == com.find("%3F", pos))
			com.replace(pos, 3, "?");
		if (pos == com.find("%40", pos))
			com.replace(pos, 3, "@");
		if (pos == com.find("%5B", pos))
			com.replace(pos, 3, "[");
		if (pos == com.find("%5C", pos))
			com.replace(pos, 3, "\\");
		if (pos == com.find("%5D", pos))
			com.replace(pos, 3, "]");
		if (pos == com.find("%5E", pos))
			com.replace(pos, 3, "^");
		if (pos == com.find("%7B", pos))
			com.replace(pos, 3, "{");
		if (pos == com.find("%7C", pos))
			com.replace(pos, 3, "|");
		if (pos == com.find("%7D", pos))
			com.replace(pos, 3, "}");
		if (pos == com.find("%7E", pos))
			com.replace(pos, 3, "~");
		if (pos == com.find("%C3%A9", pos))
			com.replace(pos, 6, "é");
		if (pos == com.find("%C3%A8", pos))
			com.replace(pos, 6, "è");
		if (pos == com.find("%C3%AA", pos))
			com.replace(pos, 6, "ê");
		if (pos == com.find("%C3%A0", pos))
			com.replace(pos, 6, "à");
		if (pos == com.find("%C3%A2", pos))
			com.replace(pos, 6, "â");
		if (pos == com.find("%C3%AE", pos))
			com.replace(pos, 6, "î");
		if (pos == com.find("%C3%AF", pos))
			com.replace(pos, 6, "ï");
		if (pos == com.find("%C3%B4", pos))
			com.replace(pos, 6, "ô");
		if (pos == com.find("%C3%B9", pos))
			com.replace(pos, 6, "ù");
		if (pos == com.find("%C3%BB", pos))
			com.replace(pos, 6, "û");
		if (pos == com.find("%C3%A7", pos))
			com.replace(pos, 6, "ç");
		if (pos == com.find("%C3%89", pos))
			com.replace(pos, 6, "É");
		if (pos == com.find("%C3%88", pos))
			com.replace(pos, 6, "È");
		if (pos == com.find("%C3%8A", pos))
			com.replace(pos, 6, "Ê");
		if (pos == com.find("%C3%80", pos))
			com.replace(pos, 6, "À");
		if (pos == com.find("%C3%8E", pos))
			com.replace(pos, 6, "Î");
		if (pos == com.find("%C3%8F", pos))
			com.replace(pos, 6, "Ï");
		if (pos == com.find("%C3%94", pos))
			com.replace(pos, 6, "Ô");
		if (pos == com.find("%C3%99", pos))
			com.replace(pos, 6, "Ù");
		if (pos == com.find("%C3%9B", pos))
			com.replace(pos, 6, "Û");
		if (pos == com.find("%C3%87", pos))
			com.replace(pos, 6, "Ç");
		if (pos == com.find("%C2%AB", pos))
			com.replace(pos, 6, "«");
		if (pos == com.find("%C2%BB", pos))
			com.replace(pos, 6, "»");
		if (pos == com.find("%E2%80%99", pos))
			com.replace(pos, 9, "’");
		pos++;
	}
}

int nbr_of_comments(const std::string path) {
	int fileCount = 0;
	DIR* dir = opendir(path.c_str());
	if (!dir)
		throw Error("Can't open comments directory.");

	if (dir == NULL)
		return -1;

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		if (entry->d_type == DT_REG) {
			++fileCount;
		}
	}
	closedir(dir);
	return fileCount;
}

void save_comment(std::string rq) {
	std::size_t p_start = rq.find("pseudo=", 0) + 7;
	std::size_t p_end = rq.find("&", p_start);
	std::size_t c_start = rq.find("comment=", p_end) + 8;
	std::size_t c_end = rq.find("\n", c_start);
	std::string pseudo = rq.substr(p_start, p_end - p_start);
	std::string comment = rq.substr(c_start, c_end - c_start);
	replace(pseudo);
	replace(comment);
	std::ostringstream filename;
	static int comment_nbr = nbr_of_comments("./site/comments/") + 1;
	filename << "./site/comments/" << comment_nbr;
	comment_nbr++;
	std::ofstream file(filename.str().c_str());
	if (file.is_open()) {
		file << pseudo << "\n\n" << comment;
		file.close();
	}
}

std::string intToString(int number) {
	std::stringstream ss;
	ss << number;
	return ss.str();
}

std::vector<std::pair<std::string, std::string> > getcomments()
{
	int n = nbr_of_comments("./site/comments/");
	int i = 1;
	std::vector<std::pair<std::string, std::string> > comments_tab;
	while(i <= n)
	{
		std::string path = "./site/comments/" + intToString(i);
		std::ifstream file;
		file.open(path.c_str());
		std::string pseudo;
		std::string comment;
		std::string line;

		std::getline(file, pseudo);
		std::getline(file, line);
		line.clear();
		while(std::getline(file, line)) {
			comment += line + "<br>";
		}
		std::pair<std::string, std::string> pair = make_pair(pseudo, comment);
		comments_tab.push_back(pair);
		i++;
	}
	return comments_tab;
}

void generate_comment_page() {
	std::vector<std::pair<std::string, std::string> > comments_tab = getcomments();
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
	oss << "		}\n";
	oss << "</style>\n";
	oss << "</head>\n";
	oss << "<body>\n";
	oss << "<h1>Comments</h1>\n";
	for(std::vector<std::pair<std::string, std::string> >::iterator i = comments_tab.begin(); i != comments_tab.end(); i++) {
		std::string pseudo = i->first;
		std::string comment = i->second;
		oss << "	<div class=\"comment\">\n";
		oss << "		<strong>" << pseudo << ":</strong><br>\n";
		oss << "		<p>" << comment << "</p>\n";
		oss << "	</div>\n";
	}
	oss << "	<h2>Leave a comment</h2>\n";
	oss << "	<form action=\"/submit_comment\" method=\"POST\">\n";
	oss << "		<label for=\"pseudo\">Pseudo:</label><br>\n";
	oss << "		<input type=\"text\" id=\"pseudo\" name=\"pseudo\" required><br><br>\n";
	oss << "		<label for=\"comment\">Comment:</label><br>\n";
	oss << "		<textarea id=\"comment\" name=\"comment\" rows=\"4\" cols=\"50\" required></textarea><br><br>\n";
	oss << "		<input type=\"submit\" value=\"Send\">\n";
	oss << "	</form>\n";
	oss << "<a href=\"/\">HOME</a>\n";
	oss << "</body>\n";
	oss << "</html>\n";
	std::ofstream page("./site/comments.html");
	if (page.is_open()) {
		page << oss.str();
		page.close();
	}
}
