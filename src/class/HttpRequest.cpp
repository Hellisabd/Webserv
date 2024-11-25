#include "HttpRequest.hpp"
#include "../../includes/webserv.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstring>
#include <iterator>
#include <map>
#include <utility>

HttpRequest::HttpRequest(string request): _request(request) {
	parsingError = false;
	fillSize();
}

HttpRequest::~HttpRequest() {
}

bool HttpRequest::isValidRequestLine() {
	size_t cur = 0;
	if (_requestSize < _minRequestSize)
		return (false);

	const char *methods[3] = {"GET ", "POST ", "DELETE "};
	bool found = false;
	string met;
	for (int i = 0; i < 3; i++) {
		if (!strncmp(_request.c_str(), methods[i], strlen(methods[i]))) {
			found = true;
			cur += strlen(methods[i]);
			break ;
		}
	}
	if (found == false) {
		return (false);
	}
	string uri;
	while (_request[cur] != ' ' && _request[cur] != '\r' && _request[cur] != '\n' && cur < _requestSize) {
		uri += _request[cur];
		cur++;
	}
	if (_request[cur] != ' ') {
		return (false);
	}
	cur++;
	if (_request.compare(cur, 8, "HTTP/1.1" )) {
		return (false);
	}
	cur += 8;
	while (isspace(_request[cur]) && _request[cur] != '\r' && _request[cur] != '\n' && cur < _requestSize) {
		cur++;
	}
	if (_request.compare(cur, 2, "\r\n" )) {
		return (false);
	}
	cur += 2;
	_requestLineSize = cur;

	// URI validation
	if (uri[0] != '/') {
		return (false); // ???????? chars valides pour l'uri ou pas
						// aucune idee de ce que l'uri peut contenir
						// ou comment elle peut etre utilisee, a creuser
	}
	return (true);
}

bool caseInsCmp(char a, char b) {
	return tolower(a) == tolower(b);
}

size_t findCaseIns(const string& str, const string& substr) {
    for (size_t i = 0; i <= str.length() - substr.length(); ++i) {
        if (equal(substr.begin(), substr.end(), str.begin() + i, caseInsCmp)) {
            return i;
        }
    }
    return string::npos;
}

string trimWhitespaces(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
	//debug(19);
    return str.substr(start, end - start + 1);
}

bool HttpRequest::isValidHost() {
	size_t hostPos = findCaseIns(_request, "host");
	if (hostPos == string::npos) {
		return (false);
	}
	if (_request.compare(hostPos - 2, 2, "\r\n")) {
		return (false);
	}
	//debug(20);
	string hostValue = _request.substr(hostPos + 5, _request.find("\r\n", hostPos) - hostPos - 5);
	hostValue = trimWhitespaces(hostValue);
	if (hostValue.size() == 0) {
		return (false);
	}
	return (true);
}

bool HttpRequest::isValid() {
	if (!isValidRequestLine()) {
		return (false);
	}
	if (!isValidHost()) {
		return (false);
	}
	return (true);
}

bool HttpRequest::parseRequest() {
	fillMethod();
	fillUrl();
	fillHostAndPort();
	fillHttpVersion();
	if (!fillHeaders())
		return (false);
	if (_method == POST) {
		if (!fillBody()) {
			return (false);
		}
	}
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
	return (_httpVersion);
}

string HttpRequest::getBody() {
	// return (string){"haha"};
	return ("haha");
}

string HttpRequest::getSpecHeader(string& spec) {
	(void)spec;
	return ("lol");
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
	size_t start = findCaseIns(_request, "host");
	size_t end = _request.find("\r\n", start);
	//debug(21);
	string hostPort = _request.substr(start + 5, end - start - 5);
	//debug(22);
	_host = hostPort.substr(0, hostPort.find(":"));
	//debug(23);
	_port = hostPort.substr(hostPort.find(":") + 1, hostPort.length());
}

void HttpRequest::fillSize() {
	_requestSize = _request.length();
}

void HttpRequest::fillMethod() {
	//debug(24);
	string met = _request.substr(0, _request.find(" "));
	if (!met.compare("GET")) {
		_method = GET;
	} else if (!met.compare("POST")) {
		_method = POST;
	} else if (!met.compare("DELETE")) {
		_method = DELETE;
	} else {
		_method = UNKNOWN;
	}
}

void HttpRequest::fillUrl() {
	size_t start = _request.find("/");
	size_t end = _request.find(" ", start);
	//debug(25);
	if (start != _request.npos && end != _request.npos)
		_url = _request.substr(start, end - start);
}

void HttpRequest::fillHttpVersion() {
	size_t start = _request.find("HTTP", 0);
	size_t end = _request.find("\r\n", start);
	if (start == _request.npos)
		start = 0;
	if (end == _request.npos)
		end = _request.length();
	//debug(26);
	_httpVersion = _request.substr(start, end - start);
	//debug(GREEN, _httpVersion);
}

string HttpRequest::extractHeaderKey(string &s) {
	pair<string, size_t> headerKey;
	headerKey.second = 0;
	for (string::iterator it = s.begin(); it != s.end(); it++) {
		if (s[*it] == ' ' || s[*it] == ':' || s[*it] == '\r' || s[*it] == '\n') {
			break ;
		}
		headerKey.second++;
	}
	//debug(27);
	headerKey.first = s.substr(0, headerKey.second);
	return (headerKey.first);
}

bool	HttpRequest::validateHeaderKey(string &headerKey) {
	size_t size = headerKey.size();
	for (size_t i = 0; i < size; i++) {
		if (headerKey[i] < 33 || headerKey[i] > 126) {
			return (false);
		}
	}
	return (true);
}

static bool	isCrlf(const string &s) {
	if (!s.compare(0, 2, "\r\n")) {
		return (true);
	}
	return (false);
}

static bool	isDoubleCrlf(const string &s) {
	if (!s.compare(0, 4, "\r\n\r\n")) {
		return (true);
	}
	return (false);
}

bool HttpRequest::fillHeaders() {
	string::iterator it = _request.begin();
	string key;
	string value;
	it += _requestLineSize;
	while (it != _request.end() && !isDoubleCrlf(&(*it))) {
		key.clear();
		value.clear();
		while (it != _request.end() && !isCrlf(&(*it)) &&  *it != ':') {
			key += *it;
			it++;
		}
		if (*it != ':') {
			parsingError = true;
			return (false);
		}
		if (!validateHeaderKey(key)) {
			parsingError = true;
			return (false);
		}
		it++;
		while (it != _request.end() && !isCrlf(&(*it)) && *it != '\r') {
			value += *it;
			it++;
		}
		if (!isCrlf(&(*it))) {
			parsingError = true;
			return (false);
		}
		if (!isDoubleCrlf(&(*it)))
			it += 2;
		value = trimWhitespaces(value);
		_header[key] = value;
	}
	if (!isDoubleCrlf(&(*it))) {
		parsingError = true;
		return (false);
	}
	return (true);
}

bool HttpRequest::fillBody() {
	return (true);
}
