#include "webserv.hpp"

string generateSessionID() {
    srand(time(0));
	ostringstream oss;
	int r = rand();
	oss << r;
    string sessionID = oss.str();
    return sessionID;
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

void replace(string &com) {
	size_t	pos = 0;
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