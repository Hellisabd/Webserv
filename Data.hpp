#pragma once

#include "webserv.hpp"

class Data {

	public:

		Data();
		Data(const Data& other);
		Data(std::string const &str);
		virtual ~Data();
		Data& operator=(const Data& other);


	private:
		int _port;
		std::string _host;
		size_t _bodySize;
		std::vector<std::string> _serverNames;

		void fill_info(std::ifstream &infile);
		void printer();
		void SetServerNames(std::string const &servernames);
};