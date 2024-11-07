#pragma once

#include <webserv.hpp>

class ServerSocket
{
private:
	std::vector<struct sockaddr_in> _address;
	std::vector<int> _sock;
	std::vector<int> _connection;
	int _backlog;
	std::vector<int> _listening;
	int _nbrPort;
public:
	ServerSocket(int domain, int service, int protocol, int *port, unsigned long interface, int backlog, int nbr_port);
	~ServerSocket();
	std::vector<struct sockaddr_in> getAddr();
	std::vector<int> getSock();
	std::vector<int> getConnection();
	int getBacklog();
	std::vector<int> getListening();
};
