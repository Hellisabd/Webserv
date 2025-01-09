#pragma once

#include <webserv.hpp>
using namespace std;


class ServerSocket
{
private:
	vector<struct sockaddr_in> 	_address;
	vector<int> 				_sock;
	vector<int> 				_connection;
	int 						_backlog;
	vector<int> 				_listening;
	int 						_nbrPort;
public:
	ServerSocket(int domain, int service, int protocol, int *port, unsigned long interface, int backlog, int nbr_port);
	~ServerSocket();
	vector<struct sockaddr_in> getAddr();
	vector<int> getSock();
	vector<int> getConnection();
	int getBacklog();
	vector<int> getListening();
};
