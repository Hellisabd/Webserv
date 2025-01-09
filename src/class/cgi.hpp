#pragma once

#include <webserv.hpp>

class cgi {
	public:
		cgi(string script, Data &data, int fd_cli, HttpRequest &rq, string request);
		char **get_argv(string script);
		void set_new_env(Data &data, HttpRequest &rq, string request);
	private:
		char 	**_argv;
		char 	**_env;
		string	_filename;
};