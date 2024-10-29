#include "HttpRequest.hpp"
#include "../includes/webserv.hpp"

HttpRequest::HttpRequest() {
	_method = UNKNOWN;
}

HttpRequest::HttpRequest(string request): _request(request) {
	_method = UNKNOWN;
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::isValid(string &request) {
	return (true);
}

bool HttpRequest::parseRequest(string& request) {
	fillSize();
	fillMethod();
	fillUrl();
	fillHostAndPort();
	fillHttpVersion();
	fillHeaders();
	fillBody();
	return (true);
}

HttpMethod	HttpRequest::getMethod() {
	return (_method);
}

string HttpRequest::getMethodToString() {
	switch (_method) {
		case GET:
			return ("GET");
		case POST:
			return ("POST");
		case DELETE:
			return ("DELETE");
		default:
			return ("UNKNOWN");
	}
}

string HttpRequest::getUrl() {
	return (_url);
}

string HttpRequest::getHttpVersion() {
	return (string){"haha"};
}

string HttpRequest::getBody() {
	return (string){"haha"};
}

string HttpRequest::getSpecHeader(string& spec) {
	return (string){"haha"};
}

map<string, string> HttpRequest::getHeaders() {
	return (_header);
}

string	HttpRequest::getHost() {
	return (_host);
}

string	HttpRequest::getPort() {
	return (_port);
}

void HttpRequest::fillHostAndPort() {
	size_t start = _request.find("Host: ");
	size_t end = _request.find("\r\n", start);
	std::string hostPort = _request.substr(start + 6, end - start - 6);
	_host = hostPort.substr(0, hostPort.find(":"));
	_port = hostPort.substr(hostPort.find(":") + 1, hostPort.length());
}

void HttpRequest::fillSize() {
	_requestSize = _request.length();
}

void HttpRequest::fillMethod() {
	std::string met = _request.substr(0, _request.find(" "));
	if (met.compare("GET")) {
		_method = GET;
	} else if (met.compare("POST")) {
		_method = POST;
	} else if (met.compare("DELETE")) {
		_method = DELETE;
	} else {
		_method = UNKNOWN;
	}
}

void HttpRequest::fillUrl() {
	size_t start = _request.find("/");
	size_t end = _request.find(" ", start);
	_url = _request.substr(start, end - start);
}

void HttpRequest::fillHttpVersion() {

}

void HttpRequest::fillHeaders() {

}

void HttpRequest::fillBody() {

}
