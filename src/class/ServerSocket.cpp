#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int domain, int service, int protocol, int port, unsigned long interface, int backlog) : _backlog(backlog)
{
	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);
	_sock = socket(domain, service, protocol);
	if (_sock < 0)
		throw Error("error in socket in ServerSocket");
	_connection = bind(_sock, (struct sockaddr *) &_address, sizeof(_address));
	if (_connection < 0)
	{
		close (_sock);
		throw Error("error in binding");
	}
	_listening = listen(_sock, _backlog);
	if (_listening < 0)
	{
		perror("Listening");
		throw Error("error in listening");
	}
}

ServerSocket::~ServerSocket()
{
	close(_sock);
}

sockaddr_in ServerSocket::getAddr()
{
	return _address;
}

int ServerSocket::getSock()
{
	return _sock;
}

int ServerSocket::getConnection()
{
	return _connection;
}

int ServerSocket::getBacklog()
{
	return _backlog;
}

int ServerSocket::getListening()
{
	return _listening;
}
