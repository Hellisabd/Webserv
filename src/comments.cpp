#include <webserv.hpp>
#include <dirent.h>

void replace(std::string &com) {
	std::size_t	pos = 0;
	while (pos < com.length())
	{
		if (pos == com.find("+", pos))
		{
			com.erase(pos, 1);
			com.insert(pos, " ");
		}
		if (pos == com.find("%0D%0A", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "\n");
		}
		if (pos == com.find("%09", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "\t");
		}
		if (pos == com.find("%3A", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, ":");
		}
		if (pos == com.find("%21", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "!");
		}
		if (pos == com.find("%22", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "\"");
		}
		if (pos == com.find("%23", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "#");
		}
		if (pos == com.find("%24", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "$");
		}
		if (pos == com.find("%25", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "%");
		}
		if (pos == com.find("%26", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "&");
		}
		if (pos == com.find("%27", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "'");
		}
		if (pos == com.find("%28", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "(");
		}
		if (pos == com.find("%29", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, ")");
		}
		if (pos == com.find("%2B", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "+");
		}
		if (pos == com.find("%2C", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, ",");
		}
		if (pos == com.find("%2F", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "/");
		}
		if (pos == com.find("%3B", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, ";");
		}
		if (pos == com.find("%3C", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "<");
		}
		if (pos == com.find("%3D", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "=");
		}
		if (pos == com.find("%3E", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, ">");
		}
		if (pos == com.find("%3F", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "?");
		}
		if (pos == com.find("%40", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "@");
		}
		if (pos == com.find("%5B", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "[");
		}
		if (pos == com.find("%5C", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "\\");
		}
		if (pos == com.find("%5D", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "]");
		}
		if (pos == com.find("%5E", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "^");
		}
		if (pos == com.find("%7B", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "{");
		}
		if (pos == com.find("%7C", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "|");
		}
		if (pos == com.find("%7D", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "}");
		}
		if (pos == com.find("%7E", pos))
		{
			com.erase(pos, 3);
			com.insert(pos, "~");
		}
		if (pos == com.find("%C3%A9", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "é");
		}
		if (pos == com.find("%C3%A8", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "è");
		}
		if (pos == com.find("%C3%AA", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "ê");
		}
		if (pos == com.find("%C3%A0", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "à");
		}
		if (pos == com.find("%C3%A2", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "â");
		}
		if (pos == com.find("%C3%AE", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "î");
		}
		if (pos == com.find("%C3%AF", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "ï");
		}
		if (pos == com.find("%C3%B4", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "ô");
		}
		if (pos == com.find("%C3%B9", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "ù");
		}
		if (pos == com.find("%C3%BB", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "û");
		}
		if (pos == com.find("%C3%A7", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "ç");
		}
		if (pos == com.find("%C3%89", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "É");
		}
		if (pos == com.find("%C3%88", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "È");
		}
		if (pos == com.find("%C3%8A", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Ê");
		}
		if (pos == com.find("%C3%80", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "À");
		}
		if (pos == com.find("%C3%8E", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Î");
		}
		if (pos == com.find("%C3%8F", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Ï");
		}
		if (pos == com.find("%C3%94", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Ô");
		}
		if (pos == com.find("%C3%99", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Ù");
		}
		if (pos == com.find("%C3%9B", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Û");
		}
		if (pos == com.find("%C3%87", pos))
		{
			com.erase(pos, 6);
			com.insert(pos, "Ç");
		}
		pos++;
	}
}

int nbr_of_comments(const std::string path) {
	int fileCount = 0;
	DIR* dir = opendir(path.c_str());

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
	replace(comment);
	debug(ORANGE, comment);
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
	oss << "</body>\n";
	oss << "</html>\n";
	std::ofstream page("./site/comments.html");
	if (page.is_open()) {
		page << oss.str();
		page.close();
	}
}
