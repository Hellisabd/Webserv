#pragma once

#include <webserv.hpp>
#include <vector>

class Data {

	public:

		Data();
		Data(const Data& other);
		Data(std::string const &str);
		virtual ~Data();
		Data& operator=(const Data& other);
		void printer();
		std::string const &getHost() const;
		int const &getPort() const;
		size_t const &getBodySize() const;
		std::vector<std::string> const &getServerNames() const;


	private:
		int _port;
		std::string _host;
		size_t _bodySize;
		std::vector<std::string> _serverNames;

		void fill_info(std::ifstream &infile);
		void SetServerNames(std::string const &servernames);
};