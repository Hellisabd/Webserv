#include "ClientSocket.hpp"


ClientSocket::ClientSocket(int domain, int service, int protocol, int port, unsigned long interface)
{
	_address.sin_family = domain;
	_address.sin_port = htons(port);
	_address.sin_addr.s_addr = htonl(interface);
	_sock = socket(domain, service, protocol);
	if (_sock < 0)
		throw Error("error in socket in ServerSocket");
	 _connection = connect(_sock, (struct sockaddr *)&_address, sizeof(_address));
	if (_connection < 0)
		throw Error("error in connecting");
}

ClientSocket::~ClientSocket()
{
	close(_sock);
}

sockaddr_in ClientSocket::getAddr()
{
	return _address;
}

int ClientSocket::getSock()
{
	return _sock;
}

int ClientSocket::getConnection()
{
	return _connection;
}
