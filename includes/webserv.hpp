#pragma once

# define MAX_EVENTS 10

#include <errno.h>
#include <netdb.h>
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
#include "../src/class/ClientSocket.hpp"
#include "../src/class/HttpRequest.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <csignal>
#include <fstream>

using namespace std;
unsigned long getFileSize(std::string const &file_path);
// enum HttpMethod {GET, POST, DELETE, UNKNOWN};

# define NC 	std::string("\e[0m")
# define RED 	std::string("\e[1;31m")
# define GREEN 	std::string("\e[1;32m")
# define YELLOW std::string("\e[1;33m")
# define ORANGE std::string("\e[1;34m")
# define PURPLE std::string("\e[1;35m")
# define BLUE 	std::string("\e[1;36m")
#define FILE_NAME (std::string(__FILE__).substr(std::string(__FILE__).find_last_of("/\\") + 1))

#define LOG(msg) std::cout << "Fichier: " << FILE_NAME << ", Fonction: " << __func__ << " -- " << msg << std::endl

class Error : public std::exception {

public :
virtual ~Error()  _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {};
Error(const std::string &msg) : _message(msg) {}
virtual const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW {return _message.c_str();}
private :
	const std::string _message;
};

#include "../tools/debug.tpp"
