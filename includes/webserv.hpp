#pragma once

# define MAX_EVENTS 100

#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <exception>
#include <iostream>
#include <fstream>
#include <cctype>
#include <sstream>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <string>
#include <vector>
#include <map>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>
#include <dirent.h>
using namespace std;

typedef struct s_requestclient {
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
	bool	cgi;
	clock_t time;
	int		pid;
	int 	fdrecv[2];
	int 	fdsend[2];
	bool	multipart;
} t_requestClient;

#include "../src/class/Data.hpp"
#include "../src/class/ServerSocket.hpp"
#include "../src/class/Epoll.hpp"
#include "../src/class/HttpRequest.hpp"
#include "../src/class/cgi.hpp"
#include "../src/class/Response.hpp"
#include "../src/class/Client.hpp"

extern int g_stop;

string generate_upload_page(vector<string> filenames);
void replace(string &com);
void delete_file(string path, Data &data);
bool check_file_availability(string rq, Data &data);
Client login(string rq, string url);
string find_filename(string request);
void	print_in_response(string headerHTTP, string tosend, int clientFD);
int		check_timeout(clock_t time);
bool 	isDir(const std::string path);
void sendDir(string &_response, string dirPath);


# define NC 	string("\e[0m")
# define RED 	string("\e[1;31m")
# define GREEN 	string("\e[1;32m")
# define YELLOW string("\e[1;33m")
# define ORANGE string("\e[1;34m")
# define PURPLE string("\e[1;35m")
# define BLUE 	string("\e[1;36m")
#define FILE_NAME (string(__FILE__).substr(string(__FILE__).find_last_of("/\\") + 1))

#define LOG(msg) cout << "Fichier: " << FILE_NAME << ", Fonction: " << __func__ << " -- " << msg << endl

class Error : public exception {

public :
virtual ~Error()  _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {};
Error(const string &msg) : _message(msg) {}
virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {return _message.c_str();}
private :
	const string _message;
};

class Disconnect : public exception {

public :
virtual ~Disconnect()  _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {};
Disconnect(const string &msg) : _message(msg) {}
virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {return _message.c_str();}
private :
	const string _message;
};



#include "../tools/debug.tpp"
