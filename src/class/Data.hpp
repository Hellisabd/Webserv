#pragma once

#include <webserv.hpp>
#include <vector>

class Data {

	public:

		Data();
		Data(const Data& other);
		Data(std::string const &str, char **env);
		virtual ~Data();
		Data& operator=(const Data& other);
		unsigned long const &getHostIP() const;
		std::string const &getHostStr() const;
		int* const &getPort() const;
		int const &getNbrPort() const;
		size_t const &getBodySize() const;
		std::map<std::string, std::string> const &getLocations() const;
		std::map<std::string, std::string> const &getErrors() const;
		std::map<std::string, std::vector<std::string>> const &getMethods() const;
		std::vector<std::string> const &getServerNames() const;
		void cpEnv(char **env);
		char **envToCharpp();
		std::map<std::string, std::string> _env;


	private:
		unsigned long _hostIP;
		std::string _hostStr;
		int *_ports;
		int _nbrPorts;
		std::string _host;
		size_t _bodySize;
		std::vector<std::string> _serverNames;
		std::map<std::string, std::string> _loc;
		std::map<std::string, std::vector<std::string>> _method;
		std::map<std::string, std::string> _errors;

		void fill_info(std::ifstream &infile);
		void SetServerNames(std::string const &servernames);
		void SetHost(std::string const &hostToShift);
		void SetPorts(std::string const &ports);
		void SetLocations(std::string const &location);
		void SetErrors(std::string const &errors);
		std::vector<std::string> setMethods(const std::string &loc);
};
std::ostream &operator<<(std::ostream &os, Data const &data);