#include "HttpRequest.hpp"
#include "../includes/webserv.hpp"
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <map>
#include <utility>

HttpRequest::HttpRequest(string request): _request(request) {
	_bodySize = -1;
	_hasContentLength = false;
	parsingStrError = "No error in sight\n";
	parsingError = false;
	_isMultipart = false;
	errno = 0;
	fillSize();
}

HttpRequest::~HttpRequest() {
}

bool caseInsCmp(char a, char b) {
	return tolower(a) == tolower(b);
}

// case insensitive strcmp, returns true if they are the same
bool caseInsStrCmp(string a, string b) {
	if (a.size() != b.size()) {
		return (false);
	}
	string::iterator ait, bit;
	ait = a.begin();
	bit = b.begin();
	while (ait != a.end() && bit != b.end()) {
		if (!caseInsCmp(*ait, *bit)) {
			return (false);
		}
		ait++, bit++;
	}
	return (true);
}

// case insensitive strncmp, returns true if they are the same
bool caseInsStrNCmp(string a, string b, size_t n) {
	string suba = a.substr(0, n);
	string subb = b.substr(0, n);
	string::iterator ait, bit;
	ait = suba.begin();
	bit = subb.begin();
	size_t i = 0;
	while (ait != suba.end() && bit != subb.end() && i < n) {
		if (!caseInsCmp(*ait, *bit)) {
			return (false);
		}
		ait++, bit++, i++;
	}
	if (i != n) {
		return (false);
	}
	return (true);
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
    return str.substr(start, end - start + 1);
}

void	HttpRequest::setErr(int n, const string& s) {
	parsingError = true;
	parsingStrError = s;
	errNo = n;
}

// returns the header key and value + true if found
// undefined + false if not found
// the search is case insensitive
const pair< const pair<string, t_headerValue> ,bool>	HttpRequest::getHeaderByKey(const string& key) {
	pair<pair<string, t_headerValue> ,bool> ret;
	for (headermap_t::iterator it = _header.begin(); it != _header.end(); it++) {
		if (caseInsStrCmp(it->first, key)) {
			ret.first.first = it->first;
			ret.first.second = it->second;
			ret.second = true;
			return (ret);
		}
	}
	ret.second = false;
	// obvious undefined first in this case;
	return (ret);
}

bool	HttpRequest::hasParameterKey(const string& paramKey, const strmap_t params) {
	for (strmap_t::const_iterator it = params.begin(); it != params.end(); it++) {
		if (caseInsStrCmp(it->first, paramKey)) {
			return (true);
		}
	}
	return (false);
}

const string HttpRequest::getParameterValue(const string& paramKey, const strmap_t params) {
	for (strmap_t::const_iterator it = params.begin(); it != params.end(); it++) {
		if (caseInsStrCmp(it->first, paramKey)) {
			return (it->second);
		}
	}
	return ("");
}

bool HttpRequest::isChunkedBasedRequest() {
	const pair<const pair<string, t_headerValue>, bool> chonk = getHeaderByKey("Transfer-Encoding");
	if (chonk.second) {
		if (caseInsStrCmp(chonk.first.second.rawValue, "chunked")) {
			setErr(501, "chunked requests not implemented, only content-length based requests.\n");
			return (true);
		}
	}
	return (false);
}

bool HttpRequest::parseRequest() {
	if (!fillMethod()) {
		return (false);
	}
	fillUrl();
	fillHostAndPort();
	fillHttpVersion();
	if (!fillHeaders())
		return (false);
	if (isChunkedBasedRequest()) {
		return (false);
	}
	// throws away request where:
	// - body is found but not content-length
	// - body is found but content-length is less that body size
	if (_hasBody) {
		const pair<const pair<string, t_headerValue>, bool> cLenHeader = getHeaderByKey("Content-Length");
		if (cLenHeader.second) {
			_hasContentLength = true;
			_contentLength = atoi(cLenHeader.first.second.rawValue.c_str());
		} else {
			setErr(501, "Body without content-length is not supported by our wonderful webserver");
			return (false);
		}
		// No longer a case of 400 bad request if the content-length is not equal. However a CL above would mean a chunked request, so i throw that one out.
		calcBodySize();
		if (_contentLength > _bodySize) {
			setErr(400, "Content-Length size is superior to the body size\n");
		}
		switch (_method) {
			case GET:
				// Ignore body with GET methods
				break ;
			case POST: {
				// has content-type ?
				const pair<headerpair_t, bool> hp = getHeaderByKey("content-type");
				if (hp.second) {
					const string& rawVal = hp.first.second.rawValue;

					// est multipart
					if (caseInsStrNCmp(rawVal, "multipart/", 10)) {
						_isMultipart = true;
						// 	- choper le type
						_multipart.type = rawVal.substr(10, rawVal.find(";"));
						_multipart.type = trimWhitespaces(_multipart.type);
						//	- valider le type
							// TODO
						// 	- choper le delimiter
						if (!hasParameterKey("boundary", hp.first.second.parameters)) {
							setErr(400, "Multipart type with no delimiter parameter\n");
							return (false);
						}
						_multipart.boundary = getParameterValue("boundary", hp.first.second.parameters);
						// valider le boundary
							// TODO
						// 	- verifier la presence des 2 prochains
						// 	  delimiters
						string body(_headerEnd, _request.end());
						// premier boundary
						cout << "--" + body.substr(0, _multipart.boundary.size()) << endl;
						if (caseInsStrNCmp("--" + body, _multipart.boundary, _multipart.boundary.size() + 2)) {
							cout << "found first boundary\n";
						} else {
							cout << "no los boundarios\n";
						}

					// n'est pas multipart
					} else {

					}
				// has no content type
				} else {

				}
				// 	- chopper entre eux
				// 		- recuperer les entetes de partie
				// 		- verifier le rnrn
				//		- si delimiter de fin, ggwp
				//		- sinon repeter
				// 	- ggwp
				// sinon
				// 	-je sais pas
				// 	- les trucs genre ?cle1=value&cle2=value
				break ;
			}
			case DELETE:
				break ;
			default:
				break ;
		}
	}
	return (true);
}

bool HttpRequest::isValidRequestLine() {
	size_t cur = 0;
	if (_requestSize < _minRequestSize) {
		setErr(400, "Request size is below the minimum size for a valid request\n");
		return (false);
	}

	const char *methods[10] = {"GET ", "POST ", "DELETE ", "POST ", "HEAD ", "OPTION ", "TRACE ", "PATCH ", "CONNECT ", "PUT "};
	bool found = false;
	string met;
	for (int i = 0; i < 10; i++) {
		if (!strncmp(_request.c_str(), methods[i], strlen(methods[i]))) {
			found = true;
			cur += strlen(methods[i]);
			break ;
		}
	}
	if (found == false) {
		setErr(400, "Couldnt find the method at the start of the request\n");
		return (false);
	}
	string uri;
	while (_request[cur] != ' ' && _request[cur] != '\r' && _request[cur] != '\n' && cur < _requestSize) {
		uri += _request[cur];
		cur++;
	}
	if (_request[cur] != ' ') {
		setErr(400, "No whitespace right afte the uri\n");
		return (false);
	}
	cur++;
	if (_request.compare(cur, 8, "HTTP/1.1" )) {
		setErr(400, "The http version is not HTTP/1.1 exactly\n");
		return (false);
	}
	cur += 8;
	while (isspace(_request[cur]) && _request[cur] != '\r' && _request[cur] != '\n' && cur < _requestSize) {
		cur++;
	}
	if (_request.compare(cur, 2, "\r\n" )) {
		setErr(400, "The request line does not end with \\r\\n");
		return (false);
	}
	cur += 2;
	_requestLineSize = cur;

	// URI validation
	if (uri[0] != '/') {
		setErr(400, "The uri does not start with /\n");
		return (false); // ???????? chars valides pour l'uri ou pas
						// aucune idee de ce que l'uri peut contenir
						// ou comment elle peut etre utilisee, a creuser
	}
	return (true);
}


bool HttpRequest::isValidHost() {
	size_t hostPos = findCaseIns(_request, "host");
	if (hostPos == string::npos) {
		setErr(400, "Couldnt find the host header\n");
		return (false);
	}
	if (_request.compare(hostPos - 2, 2, "\r\n")) {
		// attention ! n'importe quel host dans la requete va trigger validHost();
		setErr(400, "The host header is not directly after a crlf\n");
		return (false);
	}
	string hostValue = _request.substr(hostPos + 5, _request.find("\r\n", hostPos) - hostPos - 5);
	hostValue = trimWhitespaces(hostValue);
	if (hostValue.size() == 0) {
		setErr(400, "The host header does not have a value\n");
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
	return (string){"haha"};
}

string HttpRequest::getSpecHeader(string& spec) {
	(void)spec;
	return ("lol");
}

headermap_t	HttpRequest::getHeaders() {
	return (_header);
}

string	HttpRequest::getHost() {
	return (_host);
}

string	HttpRequest::getPort() {
	return (_port);
}

bool	HttpRequest::hasBody() {
	return (_hasBody);
}

size_t	HttpRequest::getBodySize() {
	return (_bodySize);
}

bool	HttpRequest::hasContentLength() {
	return (_hasContentLength);
}

size_t	HttpRequest::getContentLength() {
	return (_contentLength);
}

void HttpRequest::fillHostAndPort() {
	size_t start = findCaseIns(_request, "host");
	size_t end = _request.find("\r\n", start);
	string hostPort = _request.substr(start + 5, end - start - 5);
	_host = hostPort.substr(0, hostPort.find(":"));
	_port = hostPort.substr(hostPort.find(":") + 1, hostPort.length());
}

void HttpRequest::fillSize() {
	_requestSize = _request.length();
}

bool HttpRequest::fillMethod() {
	const string rejectedMethods[7] = {"HEAD", "PUT", "CONNECT", "OPTION", "TRACE", "PATCH"};
	string met = _request.substr(0, _request.find(" "));
	if (!met.compare("GET")) {
		_method = GET;
	} else if (!met.compare("POST")) {
		_method = POST;
	} else if (!met.compare("DELETE")) {
		_method = DELETE;
	} else {
		for (int i = 0; i < 7; i++) {
			if (!met.compare(rejectedMethods[i])) {
				setErr(405, rejectedMethods[i] + " is not allowed\n");
				return (false);
			}
		}
		_method = UNKNOWN;
		setErr(400, "Unknown method\n");
	}
	return (true);
}

void HttpRequest::fillUrl() {
	size_t start = _request.find("/");
	size_t end = _request.find(" ", start);
	_url = _request.substr(start, end - start);
}

void HttpRequest::fillHttpVersion() {
	size_t start = _request.find("HTTP");
	size_t end = _request.find("\r\n");
	_httpVersion = _request.substr(start, end - start);
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

string HttpRequest::extractHeaderKey(string &s) {
	pair<string, size_t> headerKey;
	headerKey.second = 0;
	for (string::iterator it = s.begin(); it != s.end(); it++) {
		if (s[*it] == ' ' || s[*it] == ':' || s[*it] == '\r' || s[*it] == '\n') {
			break ;
		}
		headerKey.second++;
	}
	headerKey.first = s.substr(0, headerKey.second);
	return (headerKey.first);
}

bool	HttpRequest::validateHeaderKey(string &headerKey) {
	size_t size = headerKey.size();
	for (size_t i = 0; i < size; i++) {
		if (headerKey[i] < 33 || headerKey[i] > 126) {
			setErr(400, "A char not between 33 and 126 has been found in a header key\n");
			return (false);
		}
	}
	for (headermap_t::iterator it = _header.begin(); it != _header.end(); it++) {
		if (caseInsStrCmp(headerKey, it->first)) {
			setErr(400, "A duplicate header has been found\n");
			return (false);
		}
	}
	return (true);
}

bool	HttpRequest::isEnd(const string::iterator& it) {
	return (it == _request.end() || isCrlf(&(*it)));
}

// Content-Type: multipart/form-data; boundary=------123123123 ; lol=oui
//   KEY            RAWVALUE		  PARAMKEY     PARAMVALUE    PARAM2 etc...
t_headerValue HttpRequest::extractHeaderValue(string::iterator& it) {
	string					rawValue;
	string					paramKey;
	string					paramValue;
	strmap_t				parameters;
	string					tmp;
	size_t					i = 0;

	// extracting the rawValue
	while (!isEnd(it) && *it != ';') {
		rawValue += *it;
		it++, i++;
	}
	// extracting the remaining parameters
	while (!isEnd(it)) {
		if (!parsingError && *it == ';') {
			it++;
			paramKey.clear();
			while (*it != '=' && !isEnd(it)) {
				paramKey += *it;
				it++;
			}
			if (!parsingError && *it == '=') {
				it++;
				paramValue.clear();
				while (!isEnd(it) && *it != ';') {
					paramValue += *it;
					it++, i++;
				}
			} else {
				setErr(400, "Found parameter without a clear value delimited by a =\n");
			}
			// since map Keys are const by default I have to trim here
			paramKey = trimWhitespaces(paramKey);
			parameters[paramKey] = paramValue;
		}
	}
	return ((t_headerValue){rawValue, parameters});
}

bool HttpRequest::fillHeaders() {
	string::iterator	it = _request.begin();
	string				key;

	it += _requestLineSize;
	while (it != _request.end() && !isDoubleCrlf(&(*it))) {
		key.clear();
		while (it != _request.end() && !isCrlf(&(*it)) &&  *it != ':') {
			key += *it;
			it++;
		}
		if (*it != ':') {
			setErr(400, "A header key has been found that is not followed directly by a : \n");
			return (false);
		}
		if (!validateHeaderKey(key)) {
			parsingError = true;
			return (false);
		}
		it++;
		_header[key] = extractHeaderValue(it);
		if (!isCrlf(&(*it))) {
			setErr(400, "A header is not directly followed by crlf\n");
			return (false);
		}
		if (!isDoubleCrlf(&(*it)))
			it += 2;
		_header[key].rawValue = trimWhitespaces(_header[key].rawValue);
		for (strmap_t::iterator it = _header[key].parameters.begin(); it != _header[key].parameters.end(); it++) {
			it->second = trimWhitespaces(it->second);
		}
	}
	if (!isDoubleCrlf(&(*it))) {
		setErr(400, "The header part of the request is not ended by \\r\\n\\r\\n \n");
		return (false);
	}
	it += 4;
	_headerEnd = it;
	(strlen(&(*it)) > 0) ? _hasBody = true : _hasBody = false;
	return (true);
}

void	HttpRequest::calcBodySize() {
	size_t	size = 0;
	for (string::iterator start = _headerEnd; start != _request.end(); start++) {
		size++;
	}
	_bodySize = size;
}

bool HttpRequest::fillBody() {
	return (true);
}
