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
		unsigned long const &getHostIP() const;
		std::string const &getHostStr() const;
		int* const &getPort() const;
		int const &getNbrPort() const;
		size_t const &getBodySize() const;
		std::vector<std::string> const &getServerNames() const;


	private:
		unsigned long _hostIP;
		std::string _hostStr;
		int *_ports;
		int _nbrPorts;
		std::string _host;
		size_t _bodySize;
		std::vector<std::string> _serverNames;
		std::map<std::string, std::string> _loc;
		std::map<std::string, std::string> _errors;

		void fill_info(std::ifstream &infile);
		void SetServerNames(std::string const &servernames);
		void SetHost(std::string const &hostToShift);
		void SetPorts(std::string const &ports);
		void SetLocations(std::string const &location);
		void SetErrors(std::string const &errors);
};
std::ostream &operator<<(std::ostream &os, Data const &data);