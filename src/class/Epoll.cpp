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

void Epoll::add(std::vector<struct sockaddr_in> address) {
	std::map<int, int>::iterator it = _cliport.begin();
	_noclient = false;
	for (int clientID = 0; clientID < _n; clientID++) {
		for (size_t port = 0; port < _sock.size(); port++) {
			// if (_sock[port] == 0)
			// debug(ORANGE, std::to_string(ntohs(address[port].sin_port)));
			if (it == _cliport.end() && (_cliport.size() != 0 || _noclient))
			{
				_noclient = false;
				break;
			}
			debug(YELLOW, it->second);
			debug_container(YELLOW, "_sock: ", _sock);
			if (_epollClient[clientID].data.fd == _sock[port]) {
				debug("ADDING CLIENT\n");
				debug(GREEN, "cli: ", _epollClient[clientID].data.fd);
				debug(GREEN, "serv: ", _sock[port]);
				int client = accept(_sock[port], NULL, NULL);
				if (client == -1){
					perror("Accept: ");
					throw Error("Failed to accept client connexion");
				}

				struct epoll_event new_client;
				new_client.events = EPOLLIN;
				new_client.data.fd = client;
				debug(PURPLE, client);
				if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
					close(client);
					throw Error("Error adding new client to epoll");
				}
				debug(GREEN, "New client added on port " + std::to_string(ntohs(address[port].sin_port)));
				_cliport[client] = _sock[port];
				_ClientSock.push_back(client);
				debug(PURPLE, client);
			}
			else if (it->second == _sock[port]) {
				debug("EXEC REQUEST\n");
				debug(PURPLE, "cli: ", _epollClient[clientID].data.fd);
				debug(PURPLE, "serv: ", _sock[port]);
				char buffer[1024];
				ssize_t bytes_read = 1;
				while (bytes_read > 0) {
					bytes_read = read(_epollClient[clientID].data.fd, buffer, sizeof(buffer));
					debug("bytes:read ", bytes_read);
					if (bytes_read < 0)
						break;
					if (bytes_read < 1024)
						buffer[bytes_read] = '\0';
					debug("boucle?");
					// debug(buffer);
					_HTTPRequest[clientID] += buffer;
					if (bytes_read < 1024)
						break;
				}
				debug(_HTTPRequest[clientID]);
				if (_HTTPRequest[clientID].length() == 0) {
					epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, &_epollServ);
					for (std::vector<int>::iterator fd = _ClientSock.begin(); fd != _ClientSock.end(); ++fd)
					{
						if (*fd == it->first) {
							_ClientSock.erase(fd);
							break;
						}
					}
					// if (_cliport.size() == 1)
					_noclient = true;
					close(it->first);
					it = _cliport.erase(it);
					// it = _cliport.begin();
					debug(BLUE, "Client disconnected");
				}
				else
				{
					if (_HTTPRequest[clientID].npos != _HTTPRequest[clientID].find("\r\n\r\n", 0))
						topars(_HTTPRequest[clientID]);
					std::string headerHTTP = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(getFileSize("./site/index.html")) + "\r\n\r\n";
					if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
						throw Error("Error sending HTTP header");

					int infile = open("./site/index.html", O_RDONLY);
					char tosend[1024];
					ssize_t file_read;
					while ((file_read = read(infile, tosend, sizeof(tosend))) > 0) {
						debug("file_read: ", file_read);
						send(_epollClient[clientID].data.fd, tosend, file_read, 0);
					}
					_HTTPRequest[clientID].clear();
					close(infile);
				}
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
