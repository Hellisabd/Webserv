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
	string  logMsg;
	bool 	Loged;
} t_requestClient;

class Epoll {
public:
	Epoll(vector<int> sock, int nbr_port);
	~Epoll();
	void wait(int stop);
	void handleRequest(Data &data);
	void addClient(int port);
	map<int, int>::iterator sendToClient(int clientID, Data &data, map<int, int>::iterator it);
	map<int, int>::iterator readFromClient(int clientID, map<int, int>::iterator it);
	void deleteClient(int fd);
	bool checkRequestIsValid(const string &url, Data &data, string const &method);
	bool isSockPort(int fd);
	void topars(string HTTPRequest, int clientFD);
	void modifEvents(int fd, int event, uint32_t epoll_fd);
	map<int, int>::iterator sendingFile(int fd, int infile, string headerHTTP, size_t size_to_send, map<int, int>::iterator it);
	string findRightUser(string id);
	void addLogMessage(string user, int index);
	map<int, int>::iterator	sending_upload(std::string page, int index, string id, map<int, int>::iterator it);
	void sendingToClient(int fd);

private:
	int							_epoll_fd;
	int 						_n;
	int 						_nbr_client;
	map<int, t_requestClient> 	_HTTPRequest;
	vector<int> 				_sock;
	struct epoll_event 			_epollServ;
	vector<struct epoll_event> 	_epollClient;
	map<int, int> 				_cliport;
	vector<Client> 				_ClientsData;
	clock_t 					_time_out;
	string						_response;
};