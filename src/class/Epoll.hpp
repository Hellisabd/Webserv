#pragma once
#include "webserv.hpp"
#define MAX_EVENTS 10

class Epoll {
public:
	Epoll();
	~Epoll();
private:
	int	_epoll_fd;
	std::vector<struct epoll_event> _events;
};