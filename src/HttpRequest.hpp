#include <map>
#include <string>
#include <utility>
#include <iostream>
#include "../includes/webserv.hpp"

using namespace std;

class HttpRequest {
 public:
	HttpRequest();
	HttpRequest(string request);
	~HttpRequest();
	bool				isValid(string& request);
	bool				parseRequest(string& request);
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
 private:
	// parsing methods to fill data
	void	fillMethod();
	void	fillUrl();
	void	fillHttpVersion();
	void	fillHeaders();
	void	fillBody();
	void	fillSize();
	void	fillHostAndPort();

	string				_request;
	HttpMethod			_method; // get post delete unknown
	string				_host;
	string				_port;
	string				_url;
	string				_httpVersion;
	map<string, string> _header; // host, user-agent, accept, accept-encoding, accept-language, connection
	// Host: Specifies the domain name or IP address of the server. It is required in HTTP/1.1.
	// User-Agent: Identifies the browser or client software making the request, along with the operating system.
	// Accept: Lists the media types (content types) the client can process, with optional weights (like q=0.9) to indicate preference.
	// Accept-Encoding: Specifies encoding types the client can handle, such as gzip or deflate.
	// Accept-Language: Declares the preferred language(s) for the response, here English (US) with a fallback for general English.
	// Connection: Indicates whether the client would like to keep the connection open after the request, allowing for multiple requests over a single connection.
	size_t				_requestSize;
};
