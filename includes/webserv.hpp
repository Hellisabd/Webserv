#pragma once

#include <errno.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
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
#include "../src/class/Data.hpp"
#include <iostream>
#include <vector>


# define NC std::string("\e[0m")
# define RED std::string("\e[1;31m")
# define GREEN std::string("\e[1;32m")
# define YELLOW std::string("\e[1;33m")
# define ORANGE std::string("\e[1;34m")
# define PURPLE std::string("\e[1;35m")
# define BLUE std::string("\e[1;36m")
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

template<typename T>
void	debug(std::string color, std::string msg, T content)
{
	if (msg.empty())
		std::cerr << color << content << NC << std::endl;
	else
		std::cerr << color + msg << content << NC << std::endl;
}

template<typename T>
void	debug_container(std::string color, std::string msg, T content)
{
	std::string spaces;
	for (std::size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		std::cerr << color + msg << std::endl;
	else
		std::cerr << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		std::cerr << spaces + *it << std::endl;
	std::cerr << NC;
}
