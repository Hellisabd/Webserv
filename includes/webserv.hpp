#pragma once

# define MAX_EVENTS 10

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
#include <sys/types.h>
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
#include "../src/class/Data.hpp"
#include "../src/class/ServerSocket.hpp"
#include "../src/class/Epoll.hpp"
#include "../src/class/HttpRequest.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>
#include <fstream>
#include "../src/class/Client.hpp"

using namespace std;

unsigned long getFileSize(string const &file_path);
void generate_uploads_url(vector<string> filenames);
void generate_comment_page();
void save_comment(string rq);
string uploadFile(string request, Data &data);
void replace(string &com);
void delete_file(string path, Data &data);
bool check_file_availability(string rq, Data &data);
Client login(string rq, string url);

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

#include "../tools/debug.tpp"
