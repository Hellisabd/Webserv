#pragma once

#include <webserv.hpp>
#include <vector>
using namespace std;

class Data {

	public:

		Data();
		Data(const Data& other);
		Data(string const &str, char **env);
		virtual ~Data();
		unsigned long const &getHostIP() const;
		string const &getHostStr() const;
		int* const &getPort() const;
		int const &getNbrPort() const;
		size_t const &getMaxBodySize() const;
		map<string, string> &getLocations();
		map<string, string> const &getErrors() const;
		map<string, string> const &getRedirections() const;
		map<string, vector<string> > &getMethods();
		vector<string> const &getServerNames() const;
		void cpEnv(char **env);
		char **envToCharpp();
		void fill_uploads();
		void add_upload(string filename);
		map<string, string> _env;
		vector<string> _uploads;


	private:
		unsigned long 					_hostIP;
		string 							_hostStr;
		int 							*_ports;
		int 							_nbrPorts;
		string 							_host;
		size_t 							_MaxBodySize;
		vector<string> 					_serverNames;
		map<string, string> 			_loc;
		map<string, vector<string> > 	_method;
		map<string, string> 			_errors;
		map<string, string> 			_redirect;

		void fill_info(ifstream &infile);
		void SetServerNames(string const &servernames);
		void SetLocations(string const &location);
		void SetHost(string const &hostToShift);
		void SetPorts(string const &ports);
		void SetErrors(string const &errors);
		vector<string> setMethods(const string &loc);
};
ostream &operator<<(ostream &os, Data const &data);