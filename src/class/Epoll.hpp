#pragma once
#include "webserv.hpp"
#include <ctime>

class Data;
class HttpRequest;

class Epoll {
public:
	Epoll(std::vector<int> sock, int nbr_port);
	~Epoll();
	void wait(int stop);
	void handleRequest(std::vector<struct sockaddr_in> address, Data &data);
	void addClient(int port);
	void sendToClient(int clientID, Data &data);
	void readFromClient(int clientID);
	std::map<int, int>::iterator deleteClient(std::map<int, int>::iterator it);
	void exec(Data &data, int clientID, HttpRequest rq);
	void set_new_env(Data &data, HttpRequest rq);
private:
	int	_epoll_fd;
	int _n;
	int _nbr_client;
	std::string *_HTTPRequest;
	std::vector<int> _sock;
	std::vector<int> _ClientSock;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
	std::map<int, int> _cliport;
	clock_t _time_out;
	bool _noclient;
};