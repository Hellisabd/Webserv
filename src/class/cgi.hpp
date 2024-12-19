#pragma once

#include <webserv.hpp>

class cgi {
	public:
		// ~cgi();
		cgi(string script, Data &data, int fd_cli);
		char **get_argv(string script);
	private:
		char **_argv;
		char **_env;
};