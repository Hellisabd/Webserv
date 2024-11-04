#include "Epoll.hpp"

Epoll::Epoll(std::vector<int> sock) : _sock(sock) {
	_nbr_client = 0;
	_epoll_fd = epoll_create(MAX_EVENTS);
	if (_epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
	for (int i = 0; i != NBR_PORT; i++) {
		_epollServ.events = EPOLLIN;
		_epollServ.data.fd = _sock[i];
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock[i], &_epollServ) == -1)
			throw Error("Error during epoll ctl");
	}
	_epollClient.resize(MAX_EVENTS);
	_ClientSock.resize(MAX_EVENTS);
	for (std::vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
		*i = -1;
}

Epoll::~Epoll() {
	for (int i = 0; i < _nbr_client; i++)
		close(_ClientSock[i]);
	if (_epoll_fd != -1)
		close(_epoll_fd);
}

void Epoll::wait(int stop) {
	_n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	if (_n < 0 || stop == 0) {
		close (_epoll_fd);
		for (std::vector<int>::iterator i = _sock.begin(); i != _sock.end(); i++)
			close (*i);
		throw Error("Error during epoll_wait");
	}
}

void topars(std::string HTTPRequest)
{
	(void)HTTPRequest;
}

void Epoll::addClient(int port)
{
	int client = accept(_sock[port], NULL, NULL);
	if (client == -1){
		perror("Accept: ");
		throw Error("Failed to accept client connexion");
	}
	struct epoll_event new_client;
	new_client.events = EPOLLIN;
	new_client.data.fd = client;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
		close(client);
		throw Error("Error adding new client to epoll");
	}
	_cliport[client] = _sock[port];
	_ClientSock.push_back(client);
}

void Epoll::sendToClient(int clientID) {
	if (_HTTPRequest[clientID].npos != _HTTPRequest[clientID].find("\r\n\r\n", 0))
		topars(_HTTPRequest[clientID]);
	std::string headerHTTP = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(getFileSize("./site/index.html")) + "\r\n\r\n";
	if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");

	int infile = open("./site/index.html", O_RDONLY);
	char tosend[1024];
	ssize_t file_read;
	while ((file_read = read(infile, tosend, sizeof(tosend))) > 0) {
		if (file_read < 1024)
			tosend[file_read++] = '\0';
		if (send(_epollClient[clientID].data.fd, tosend, file_read, MSG_NOSIGNAL) < 0)
		{
			close(infile);
			perror("send body");
			throw Error("");
		}
	}
	_HTTPRequest[clientID].clear();
	close(infile);
}

void Epoll::readFromClient(int clientID)
{
	char buffer[1024];
	ssize_t bytes_read = 1;
	while (bytes_read > 0) {
		bytes_read = read(_epollClient[clientID].data.fd, buffer, sizeof(buffer));
		if (bytes_read < 0)
			break;
		if (bytes_read < 1024)
			buffer[bytes_read] = '\0';
		_HTTPRequest[clientID] += buffer;
		if (bytes_read < 1024)
			break;
	}
}

std::map<int, int>::iterator Epoll::deleteClient(std::map<int, int>::iterator it) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, &_epollServ);
	for (std::vector<int>::iterator fd = _ClientSock.begin(); fd != _ClientSock.end(); ++fd)
	{
		if (*fd == it->first) {
			_ClientSock.erase(fd);
			debug(RED, "Client disconnected.");
			break;
		}
	}
	_noclient = true;
	close(it->first);
	return (_cliport.erase(it));
}

void Epoll::handleRequest(std::vector<struct sockaddr_in> address) {
	std::map<int, int>::iterator it = _cliport.begin();
	_noclient = false;
	for (int clientID = 0; clientID < _n; clientID++) {
		for (size_t port = 0; port < _sock.size(); port++) {
			if (it == _cliport.end() && (_cliport.size() != 0 || _noclient))
			{
				_noclient = false;
				break;
			}
			if (_epollClient[clientID].data.fd == _sock[port])
			{
				addClient(port);
				debug(GREEN, "New client added on port " + std::to_string(ntohs(address[port].sin_port)));
			}
			else if (it->second == _sock[port]) {
				
				readFromClient(clientID);
				if (_HTTPRequest[clientID].length() == 0)
					it = deleteClient(it);
				else
					sendToClient(clientID);
			}
			if (_noclient == true)
			{
				_noclient = false;
				break;
			}
		}
		if (it == _cliport.end())
			break;
		++it;
	}
}
