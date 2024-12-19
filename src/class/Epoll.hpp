#pragma once
#include <webserv.hpp>
// #include "Client.hpp"
#include <ctime>
using namespace std;


class Data;
class HttpRequest;
class Client;
class cgi;

typedef struct s_requestclient
{
	string 	req;
	string 	body;
	int 	nbr_of_read;
	bool 	recvEnd;
	bool 	sendEnd;
	bool 	disconnect;
	size_t 	bodysize;
	size_t 	size_to_reach;
	size_t 	size_of_file_to_send;
	bool 	sending;
	string 	headerresponse;
	int 	infile;
	string 	connectionType;
	string 	id;
	string 	page;
	bool	uploading;
} t_requestClient;

class Epoll {
public:
	Epoll(vector<int> sock, int nbr_port);
	~Epoll();
	void wait(int stop);
	void handleRequest(Data &data);
	void addClient(int port);
	map<int, int>::iterator sendToClient(int clientID, Data &data, map<int, int>::iterator it);
	void readFromClient(int clientID);
	map<int, int>::iterator deleteClient(map<int, int>::iterator it);
	map<int, int>::iterator exec(Data &data, int clientID, HttpRequest rq, string req_str, map<int, int>::iterator it, string id);
	void set_new_env(Data &data, HttpRequest rq);
	bool checkRequestIsValid(const string &url, Data &data, string const &method);
	void downloadFile(string request);
	bool isSockPort(int fd);
	void topars(string HTTPRequest, int clientFD);
	void modifEvents(int fd, int event, int epoll_fd);
	map<int, int>::iterator sendingFile(int fd, int infile, string headerHTTP, size_t size_to_send, map<int, int>::iterator it, string id);
	bool check_ID(int id);
	string findRightUser(string id);

private:
	int	_epoll_fd;
	int _n;
	int _nbr_client;
	map<int, t_requestClient> _HTTPRequest;
	vector<int> _sock;
	vector<int> _ClientSock;
	struct epoll_event _epollServ;
	vector<struct epoll_event> _epollClient;
	map<int, int> _cliport;
	vector<Client> _ClientsData;
	clock_t _time_out;
	bool _noclient;
};