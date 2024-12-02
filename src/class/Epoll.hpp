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
	std::size_t size_of_file_to_send;
	bool sending;
	std::string headerresponse;
	int infile;
	std::string connectionType;
} t_requestClient;

class Epoll {
public:
	Epoll(std::vector<int> sock, int nbr_port);
	~Epoll();
	void wait(int stop);
	void handleRequest(std::vector<struct sockaddr_in> address, Data &data);
	void addClient(int port);
	std::map<int, int>::iterator sendToClient(int clientID, Data &data, std::map<int, int>::iterator it);
	void readFromClient(int clientID);
	std::map<int, int>::iterator deleteClient(std::map<int, int>::iterator it);
	std::map<int, int>::iterator exec(Data &data, int clientID, HttpRequest rq, std::string req_str, std::map<int, int>::iterator it);
	void set_new_env(Data &data, HttpRequest rq);
	bool checkRequestIsValid(const std::string &url, Data &data, std::string const &method);
	void downloadFile(std::string request);
	bool isSockPort(int fd);
	void topars(std::string HTTPRequest, int clientFD);
	void modifEvents(int fd, int event, int epoll_fd);
	std::map<int, int>::iterator sendingFile(int fd, int infile, std::string headerHTTP, std::size_t size_to_send, std::map<int, int>::iterator);


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