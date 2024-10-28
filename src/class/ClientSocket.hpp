#pragma once

#include <webserv.hpp>

class ClientSocket
{
private:
	struct sockaddr_in _address;
	int _sock;
	int _connection;
public:
	ClientSocket(int domain, int service, int protocol, int port, unsigned long interface);
	~ClientSocket();
	struct sockaddr_in getAddr();
	int getSock();
	int getConnection();
};