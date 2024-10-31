#pragma once
#include "webserv.hpp"

class Epoll {
public:
	Epoll(std::vector<int> sock);
	~Epoll();
	void wait(int stop);
	void add(std::vector<struct sockaddr_in> address);
private:
	int	_epoll_fd;
	int _n;
	int _nbr_client;
	std::string _HTTPRequest[MAX_EVENTS];
	std::vector<int> _sock;
	std::vector<int> _ClientSock;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
	std::map<int, int> _cliport;

};