#pragma once
#include <webserv.hpp>
#include <ctime>

class Data;
class HttpRequest;

typedef struct s_requestclient
{
	std::string req;
	std::string body;
	int nbr_of_read;
	bool recvEnd;
	bool sendEnd;
	bool disconnect;
	std::size_t bodysize;
	std::size_t size_to_reach;
} t_requestClient;

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
	void exec(Data &data, int clientID, HttpRequest rq, std::string req_str);
	void set_new_env(Data &data, HttpRequest rq);
	bool checkRequestIsValid(const std::string &url, Data &data, std::string const &method);
	void downloadFile(std::string request);
	bool isSockPort(int fd);
	void topars(std::string HTTPRequest, int clientFD);
	void modifEvents(int fd, int event, int epoll_fd);


private:
	int	_epoll_fd;
	int _n;
	int _nbr_client;
	std::map<int, t_requestClient> _HTTPRequest;
	std::vector<int> _sock;
	std::vector<int> _ClientSock;
	struct epoll_event _epollServ;
	std::vector<struct epoll_event> _epollClient;
	std::map<int, int> _cliport;
	clock_t _time_out;
	bool _noclient;
};