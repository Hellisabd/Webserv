#pragma once
#include "webserv.hpp"
#define MAX_EVENTS 10

class Epoll {
public:
	Epoll(std::vector<int> sock);
	~Epoll();
	int wait(int stop, std::vector<int> sock);
	int add(std::vector<int> sock);
private:
	int	_epoll_fd;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
};