#include "webserv.hpp"

void generate_login_result_page(int n) {
	ostringstream oss;
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
	ofstream page("./site/try_login.html");
	if (page.is_open()) {
		page << oss.str();
		page.close();
	}
}

void encrypt_pw(string &pw, string user) {
	const char *base = "0123456789abcdef";
	int i = 0;
	int size = user.length();
	for (string::iterator it = pw.begin(); it != pw.end(); it++) {
		*it = (*it) * size * base[i % 16];
		i++;
	}
}

string generateSessionID() {
    srand(time(0));
	ostringstream oss;
	int r = rand();
	oss << r;
    string sessionID = oss.str();
    return sessionID;
}

bool pseudo_available(string pseudo) {
	string line;
	ifstream file;
	file.open("./site/users/users");
	while(getline(file, line)) {
		size_t lim = line.find(":");
		string user = line.substr(0, lim);
		if (pseudo == user)
			return false;
	}
	return true;
}

bool check_login(string pseudo, string password) {
	string line;
	ifstream file;
	file.open("./site/users/users");
	while(getline(file, line)) {
		size_t lim = line.find(":");
		string user = line.substr(0, lim);
		string pw = line.substr(lim + 1, line.length() - lim + 1);
		if (pseudo == user && password == pw)
			return true;
	}
	return false;
}

void add_user(string pseudo, string password) {
	ostringstream filename;
	filename << "./site/users/users";
	ofstream file(filename.str().c_str(), ios::app);
	if (file.is_open()) {
		file << pseudo << ":" << password << endl;
		file.close();
	}
}

vector<string> pars_login(string rq) {
	size_t p_start;
	size_t p_end;
	size_t pw_start;
	size_t pw_end;
	string pseudo;
	string password;
	if (rq.find("pseudo") != rq.npos) {
		p_start = rq.find("pseudo=", 0) + 7;
		p_end = rq.find("&", p_start);
		pw_start = rq.find("password=", p_end) + 9;
		pw_end = rq.find("\n", pw_start);
		pseudo = rq.substr(p_start, p_end - p_start);
		password = rq.substr(pw_start, pw_end - pw_start);
		replace(pseudo);
		replace(password);
		encrypt_pw(password, pseudo);
		if (check_login(pseudo, password) == true)
			generate_login_result_page(1);
		else {
			pseudo.clear();
			generate_login_result_page(2);
		}
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
			encrypt_pw(password, pseudo);
			add_user(pseudo, password);
			generate_login_result_page(3);
		}
		else {
			generate_login_result_page(4);
			pseudo.clear();
		}
	}
	vector<string> vec;
	vec.push_back(pseudo);
	vec.push_back(password);
	vec.push_back(generateSessionID());
	return vec;
}

Client login(string rq, string url) {
	vector<string> login_id;
	if (url.find("/try_login") != url.npos)
		login_id = pars_login(rq);
	return(Client(login_id));
}