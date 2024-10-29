#include "Epoll.hpp"

Epoll::Epoll(std::vector<int> sock) {
	_epoll_fd = epoll_create(MAX_EVENTS);
	if (_epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
	for (int i = 0; i != NBR_PORT; i++) {
		_epollServ.events = EPOLLIN;
		_epollServ.data.fd = sock[i];
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock[i], &_epollServ) == -1)
			throw Error("Error during epoll ctl");
	}
}

Epoll::~Epoll() {
	if (_epoll_fd != -1)
		close(_epoll_fd);
}

int Epoll::wait(int stop, std::vector<int> sock) {
	int n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	if (n < 0 || stop == 0) {
		close (_epoll_fd);
		for (std::vector<int>::iterator i = sock.begin(); i != sock.end(); i++)
			close (*i);
		throw Error("Error during epoll_wait");
	}
}

int Epoll::add(std::vector<int> sock) {
	
}