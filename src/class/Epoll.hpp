#pragma once
#include "webserv.hpp"
#define MAX_EVENTS 10

class Epoll {
public:
	Epoll(int sock);
	~Epoll();
	int wait();
	int add();
private:
	int	_epoll_fd;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
};