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