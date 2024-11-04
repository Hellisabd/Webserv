#include "ServerSocket.hpp"

ServerSocket::ServerSocket(int domain, int service, int protocol, int *port, unsigned long interface, int backlog, int nbr_port) : _backlog(backlog), _nbrPort(nbr_port)
{
	_address.resize(_nbrPort);
	for (int i = 0; i != _nbrPort; i++)
	{
		_address[i].sin_family = domain;
		_address[i].sin_port = htons(port[i]);
		_address[i].sin_addr.s_addr = htonl(interface);
		int sock = socket(domain, service, protocol);
		int opt = 1;
		if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw Error("Reuse adress");
		_sock.push_back(sock);
		if (_sock[i] < 0)
			throw Error("error in socket in ServerSocket");
		_connection.push_back(bind(_sock[i], (struct sockaddr *) &_address[i], sizeof(_address)));
		if (_connection[i] < 0) {
			close (_sock[i]);
			perror("Error");
			throw Error("error in binding");
		}
		_listening.push_back(listen(_sock[i], _backlog));
		if (_listening[i] < 0) {
			perror("Listening");
			close (_sock[i]);
			throw Error("error in listening");
		}
	}
}

ServerSocket::~ServerSocket() {
	for (int i = 0; i != _nbrPort; i++) {
		close(_sock[i]);
	}
}

std::vector<sockaddr_in> ServerSocket::getAddr() {
	return _address;
}

std::vector<int> ServerSocket::getSock() {
	return _sock;
}

std::vector<int> ServerSocket::getConnection() {
	return _connection;
}

int ServerSocket::getBacklog() {
	return _backlog;
}

std::vector<int> ServerSocket::getListening() {
	return _listening;
}
