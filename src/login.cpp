#include "webserv.hpp"

void generate_login_result_page(int n) {
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
	if (n == 1) {
		oss << "	<p>You were successfully logged! Welcome back!</p>\n";
	}
	else if (n == 2) {
		oss << "	<p>The given user name or password was wrong! Please try again.</p>\n";
	}
	else if (n == 3) {
		oss << "	<p>Your user account was successfully created!</p>\n";
	}
	else if (n == 4) {
		oss << "	<p>The given user name is already used, try a new one.</p>\n";
	}
	oss << "<a href=\"/login\">back</a>\n";
	oss << "</body>\n";
	oss << "</html>\n";
	std::ofstream page("./site/try_login.html");
	if (page.is_open()) {
		page << oss.str();
		page.close();
	}
}

bool pseudo_available(std::string pseudo) {
	std::string line;
	std::ifstream file;
	file.open("./site/users/users");
	while(std::getline(file, line)) {
		std::size_t lim = line.find(":");
		std::string user = line.substr(0, lim);
		if (pseudo == user)
			return false;
	}
	return true;
}

bool check_login(std::string pseudo, std::string password) {
	std::string line;
	std::ifstream file;
	file.open("./site/users/users");
	while(std::getline(file, line)) {
		std::size_t lim = line.find(":");
		std::string user = line.substr(0, lim);
		std::string pw = line.substr(lim + 1, line.length() - lim + 1);
		if (pseudo == user && password == pw)
			return true;
	}
	return false;
}

void add_user(std::string pseudo, std::string password) {
	std::ostringstream filename;
	filename << "./site/users/users";
	std::ofstream file(filename.str().c_str(), std::ios::app);
	if (file.is_open()) {
		file << pseudo << ":" << password << std::endl;
		file.close();
	}
}

void pars_login(std::string rq) {
	std::size_t p_start;
	std::size_t p_end;
	std::size_t pw_start;
	std::size_t pw_end;
	std::string pseudo;
	std::string password;
	if (rq.find("pseudo") != rq.npos) {
		p_start = rq.find("pseudo=", 0) + 7;
		p_end = rq.find("&", p_start);
		pw_start = rq.find("password=", p_end) + 9;
		pw_end = rq.find("\n", pw_start);
		pseudo = rq.substr(p_start, p_end - p_start);
		password = rq.substr(pw_start, pw_end - pw_start);
		replace(pseudo);
		replace(password);
		if (check_login(pseudo, password) == true)
			generate_login_result_page(1);
		else
			generate_login_result_page(2);
	}
	if (rq.find("new_user") != rq.npos) {
		p_start = rq.find("new_user=", 0) + 9;
		p_end = rq.find("&", p_start);
		pw_start = rq.find("password=", p_end) + 9;
		pw_end = rq.find("\n", pw_start);
		pseudo = rq.substr(p_start, p_end - p_start);
		password = rq.substr(pw_start, pw_end - pw_start);
		replace(pseudo);
		replace(password);
		if (pseudo_available(pseudo)) {
			add_user(pseudo, password);
			generate_login_result_page(3);
		}
		else
			generate_login_result_page(4);
	}
}

void login(std::string rq, std::string url) {
	if (url.find("/try_login") != url.npos)
		pars_login(rq);
}