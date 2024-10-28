#pragma once

#include <webserv.hpp>

class ServerSocket
{
private:
	struct sockaddr_in _address;
	int _sock;
	int _connection;
	int _backlog;
	int _listening;
public:
	ServerSocket(int domain, int service, int protocol, int port, unsigned long interface, int backlog);
	~ServerSocket();
	struct sockaddr_in getAddr();
	int getSock();
	int getConnection();
	int getBacklog();
	int getListening();
};
