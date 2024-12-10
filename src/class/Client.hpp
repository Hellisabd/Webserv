#pragma once

#include <webserv.hpp>
using namespace std;

class Client
{
private:
	string _id;
	string _pw;
	string _user;
public:
	Client(vector<string> user_pw_id);
	Client &operator=(const Client &other);
	~Client();
	string const &getUser();
	string const &getPassword();
	string const &getID();
	bool checkLog(string username);
};

