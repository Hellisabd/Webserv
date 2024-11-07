#pragma once

#include <map>
#include <string>
#include <utility>
#include <iostream>
#include "../../includes/webserv.hpp"

using namespace std;


enum HttpMethod {GET, POST, DELETE, UNKNOWN};

// Usage : request in the constructor,
// .isValid() first,
// then parseRequest
// then check .parsingError;
class HttpRequest {
 public:
	HttpRequest(string request);
	~HttpRequest();
	bool				isValid();
	bool				isValidRequestLine();
	bool				isValidHost();
	bool				parseRequest();
	string				getMethodToString();
	HttpMethod			getMethod();
	string				getUrl();
	string				getHttpVersion();
	string				getBody();
	map<string, string>	getHeaders();
	string				getSpecHeader(string& spec);
	size_t				getSize();
	string				getHost();
	string				getPort();

	bool				parsingError;
 private:
	static const size_t _minRequestSize = 18; // GET / HTTP/1.1\\n\r\n
	// parsing methods to fill data
	void	fillMethod();
	void	fillUrl();
	void	fillHttpVersion();
	bool	fillHeaders();
	bool	fillBody();
	void	fillSize();
	void	fillHostAndPort();

	//utils
	string	extractHeaderKey(std::string &s);
	bool	validateHeaderKey(std::string& headerKey);

	// request slices
	string				_request;
	size_t				_requestLineSize;
	HttpMethod			_method; // get post delete unknown
	string				_host;
	string				_port;
	string				_url;
	string				_httpVersion;
	map<string, string> _header;
	size_t				_requestSize;
};
