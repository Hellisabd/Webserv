#pragma once
#include "webserv.hpp"
#define MAX_EVENTS 10

class Epoll {
public:
	Epoll(std::vector<int> sock);
	~Epoll();
	void wait(int stop);
	void add();
private:
	int	_epoll_fd;
	std::vector<int> _sock;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
};