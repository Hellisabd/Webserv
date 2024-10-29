#include "Epoll.hpp"

Epoll::Epoll(int sock) {
	_epoll_fd = epoll_create(MAX_EVENTS);
	if (_epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
	_epollServ.events = EPOLLIN;
	_epollServ.data.fd = sock;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, sock, &_epollServ) == -1)
		throw Error("Error during epoll ctl");
}

Epoll::~Epoll() {
	if (_epoll_fd != -1)
		close(_epoll_fd);
}

int Epoll::wait()
{

}