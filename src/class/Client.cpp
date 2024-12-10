#include "Client.hpp"

Client::Client(vector<string> user_pw_id)
{
	vector<string>::iterator it = user_pw_id.begin();
	_user = *it;
	++it;
	_pw = *it;
	++it;
	_id = *it;
}

Client &Client::operator=(const Client &other) {
	if (this != &other) {
		_user = other._user;
		_pw = other._pw;
		_id = other._id;
	}
	return *this;
}

string const &Client::getUser() {
	return _user;
}

string const &Client::getPassword() {
	return _pw;
}

string const &Client::getID() {
	return _id;
}

Client::~Client()
{
}

bool Client::checkLog(string username)
{
	if (username == _user)
		return true;
	return false;
}