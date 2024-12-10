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
	_isUrlEncoded = false;
	errno = 0;
	fillSize();
}

HttpRequest::~HttpRequest() {
}

//------------ UTILS ------------ //

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

static bool caseInsCmp(char a, char b) {
	return tolower(a) == tolower(b);
}

// case insensitive strcmp, returns true if they are the same
static bool caseInsStrCmp(string a, string b) {
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
static bool caseInsStrNCmp(string a, string b, size_t n) {
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

static size_t findCaseIns(const string& str, const string& substr) {
    for (size_t i = 0; i <= str.length() - substr.length(); ++i) {
        if (equal(substr.begin(), substr.end(), str.begin() + i, caseInsCmp)) {
            return i;
        }
    }
    return string::npos;
}

static string trimWhitespaces(const string& str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == string::npos) {
        return "";
    }
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, end - start + 1);
}

// ------------ END UTILS ---------- //

void	HttpRequest::setErr(int n, const string& s) {
	parsingError = true;
	parsingStrError = s;
	errNo = n;
}

bool HttpRequest::parseAll() {
	if (!parseHeader()) {
		return (false);
	}
	if (!parseBody()) {
		return (false);
	}
	return (true);
}

bool HttpRequest::parseHeader() {
	if (!isValid()) {
		return (false);
	}
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
	return (true);
}

bool HttpRequest::parseBody() {
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
			// debug(_contentLength);
			// debug(_bodySize);
			setErr(400, "Content-Length size is superior to the body size\n");
			return (false);
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
						if (!isValidBoundary()) {
							return (false);
						}
						// parser les boundaries:
						//	- verifier que chaquns sont valides
						//		- le premier est precede de \r\n
						//		(compris comme inclus dans le body de la requete),
						//		suivis de \r\n, ne doit pas apparaitre
						//		sauvagement dans le body.
						//	- verifier que le dernier soit affixe de --
						//	- compter le nombre
						//	- split les content dans la struct multipart
						if (!allBoundaryAreValid()) {
							return (false);
						}
						if (!extractMultiparts()) {
							return (false);
						}
						if (!extractMultipartHeaders()) {
							return (false);
						}
						if (!validContentType()) {
							return (false);
						}
						if (!extractMultipartFiles()) {
							return (false);
						}
					// est urlencoded
					} else if (caseInsStrNCmp(rawVal, "application/x-www-urlencoded", 27)){
						_isUrlEncoded = true;
						_urlEncodedBody = &(*_headerEnd);
					} else {
						// TODO ????
					}
				// has no content type
				} else {
					// TODO ????
				}
				break ;
			}
			case DELETE:
				// ignore body with delete methods
				break ;
			default:
				break ;
		}
	}
	return (true);
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

bool	isInBoundarySpecialCharset(int n) {
	const char *set = {"'()+_,-/:=?"};
	for (int i = 0; i < 12; i++) {
		if (n == set[i]) {
			return (true);
		}
	}
	return (false);
}

//		- le format valide (2046 5.1.1)
//		 boundary := 0*69<bchars> bcharsnospace
//		 bchars := bcharsnospace / " "
// 		 bcharsnospace := DIGIT / ALPHA  '  (  )
//		 +  _  ,  -  . /  :  =  ?
bool	HttpRequest::isValidBoundary() {
	const string bd = _multipart.boundary;
	if (bd.size() >= 70) {
		setErr(400, "Boundary is more than 70 bytes\n");
		return (false);
	}
	for (string::const_iterator it = bd.begin(); it != bd.end(); it++) {
		if (!isalnum(*it) && !isInBoundarySpecialCharset(*it)) {
			setErr(400, "Character is not allowed in a boundary\n");
			return (false);
		}
	}
	return (true);
}

bool	HttpRequest::allBoundaryAreValid() {
	string				b = _multipart.boundary;
	string				lastB;
	string::iterator	bit = _headerEnd;
	size_t				bsize = b.size();
	size_t				bpos;
	size_t				bnb = 0;

	string::iterator lol = _request.end();
	for (int i = 0; i < 50; i++)
	{
		lol--;
	}
	cout << endl;
	while ((bpos = static_cast<string>(&(*bit)).find(b)) != string::npos) {
		if (bpos != string::npos) {
			// TODO check premier boudary et son inclusion au \r\b de fin de header (enfin je sais pas a check)
			if (static_cast<string>(&(*(bit + bpos - 2))).compare(0, 2, "--")) {
				setErr(400, "Found a boundary delimiter not prefixed with --");
				return (false);
			} else if (static_cast<string>(&(*(bit + bpos))).compare(bsize, 2, "\r\n") && static_cast<string>(&(*(bit + bpos))).compare(bsize, 4, "--\r\n")) {
				// TODO, check ca
				setErr(400, "Found a boundary delimiter not immediately followed by \\r\\n. (maybe the RFC states that there can be whitespaces at the end, but im not trusting a random stack overflow comment and im too lazy to check right now.)\n");
				return (false);
			} else if (!static_cast<string>(&(*(bit + bpos - 2))).compare(0, bsize + 4, "--" + b + "--")) {
				lastB = &(*(bit + bpos - 2));
				cout << "alsdjflasdf" << endl;
				break ;
			}
			lastB = &(*bit);
			bpos -= 2;
			bnb++;
			bit += bpos + bsize;
		}
	}
	if (bnb == 0) {
		setErr(400, "No delimiter found inside the request body\n");
		return (false);
	} else if (lastB.compare(0 , bsize + 4,"--" + b + "--")) {
		//cout << "test: " << lastB << endl;
		setErr(400, "The last boundary delimiter is not suffixed with --");
		return (false);
	}
	_multipart.partNb = bnb;
	return (true);
}

bool HttpRequest::extractMultiparts() {
	string::iterator	bit = _headerEnd;
	string				b = _multipart.boundary;
	size_t				bsize = _multipart.boundary.size();
	size_t				bpos;

	for (size_t i = 0; i < _multipart.partNb; i++) {
		bpos = static_cast<string>(&(*bit)).find(b);
		bit += bpos + bsize;
		_multipart.partsContents.push_back(static_cast<string>(&(*bit)).substr(0, static_cast<string>(&(*bit)).find(b) - 2));
	}
	return (true);
}

bool	HttpRequest::validateMultipartHeaderKey(string &headerKey, headermap_t hm) {
	size_t size = headerKey.size();

	for (size_t i = 0; i < size; i++) {
		if (headerKey[i] < 33 || headerKey[i] > 126) {
			setErr(400, "A char not between 33 and 126 has been found in a header key\n");
			return (false);
		}
	}
	for (headermap_t::iterator it = hm.begin(); it != hm.end(); it++) {
		if (caseInsStrCmp(headerKey, it->first)) {
			setErr(400, "A duplicate header has been found\n");
			return (false);
		}
	}
	return (true);
}

bool HttpRequest::isMultipartHeaderRightfullyFormatted(const string& s) {
	string::const_iterator it = s.begin();

	while (!isDoubleCrlf(&(*it))) {
		if (!_multipart.boundary.compare(&(*it))) {
			setErr(400, "something about the boundary delimiter found inside a multipart header (tip: dont do that)\n");
		} else if (it == s.end()) {
			setErr(400, "something about a multipart header wrongfully reaching the end of its part (tip: double marine le pen)\n");
		}
		it++;
	}
	return (true);
}

bool HttpRequest::extractMultipartHeaders() {
	vector<string>	pc = _multipart.partsContents;

	for (size_t i = 0; i < pc.size(); i++) {
		if (!isMultipartHeaderRightfullyFormatted(pc[i])) {
			parsingStrError = true;
			return (false);
		}
		string				hd = pc[i].substr(0, pc[i].find("\r\n\r\n") + 4);
		string::iterator	it = hd.begin() + 2;
		string				key;
		headermap_t			hm;

		_multipart.headerLens.push_back(hd.size());
		while (it != hd.end() && !isDoubleCrlf(&(*it))) {
			key.clear();
			while (it != hd.end() && !isCrlf(&(*it)) && *it != ':') {
				key += *it;
				it++;
			}
			if (*it != ':') {
				setErr(400, "A header key has been found that is not followed directly by a :\n");
				return (false);
			}
			if (!validateMultipartHeaderKey(key, hm)) {
				parsingError = true;
				return (false);
			}
			it++;
			hm[key] = extractHeaderValue(it);
			if (!isCrlf(&(*it))) {
				setErr(400, "A header is not directly followed by crlf\n");
				return (false);
			}
			if (!isDoubleCrlf(&(*it))) {
				it += 2;
			}
			hm[key].rawValue = trimWhitespaces(hm[key].rawValue);
			for (strmap_t::iterator it2 = hm[key].parameters.begin(); it2 != hm[key].parameters.end(); it2++) {
				it2->second = trimWhitespaces(it2->second);
			}
		}
		if (!isDoubleCrlf(&(*it))) {
			setErr(400, "The header part of the request is not ended by \\r\\n\\r\\n \n");
			return (false);
		}
		_multipart.headers.push_back(hm);
	}
	return (true);
}

bool HttpRequest::extractMultipartFiles() {
	vector<string> pc = _multipart.partsContents;

	for (size_t i = 0; i < pc.size(); i++) {
		_multipart.partsFiles.push_back(pc[i].substr(_multipart.headerLens[i], pc[i].size()));
	}
	return (true);
}

bool	HttpRequest::validContentType() {
	vector<headermap_t> mhd = _multipart.headers;

	for (vector<headermap_t>::iterator it = mhd.begin(); it != mhd.end(); it++) {
		bool	found = false;
		for (headermap_t::iterator it2 = it->begin(); it2 != it->end(); it2++) {
			if (caseInsStrCmp(it2->first, "content-type")) {
				found = true;
				if (!caseInsStrCmp(it2->second.rawValue, "application/octet-stream") && !caseInsStrCmp(it2->second.rawValue, "text/plain")) {
					setErr(501, it2->second.rawValue + " is not a supported file content-type\n");
				}
			}
		}
		if (!found) {
			setErr(400, "Multipart type without a content-type for one or more files\n");
			return (false);
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
	_httpVersion = _request.substr(start, end - start);
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
		else
			break;
	}
	return ((t_headerValue){rawValue, parameters});
}

t_headerValue HttpRequest::extractUserAgent(string::iterator& it) {
	string					rawValue;
	string					paramKey;
	string					paramValue;
	strmap_t				parameters;
	string					tmp;
	size_t					i = 0;

	// extracting the rawValue
	while (!isEnd(it)) {
		rawValue += *it;
		it++, i++;
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
		if (caseInsStrCmp(key, "user-agent")) {
			_header[key] = extractUserAgent(it);
		} else {
			_header[key] = extractHeaderValue(it);
		}
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

HttpMethod	HttpRequest::getMethod() const {
	return (_method);
}

string HttpRequest::getMethodToString() const {
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

string HttpRequest::getUrl() const {
	return (_url);
}

string HttpRequest::getHttpVersion() const {
	return (_httpVersion);
}

string HttpRequest::getBody() const {
	return  static_cast<string>("haha");
}

string HttpRequest::getSpecHeader(string& spec) const {
	(void)spec;
	return ("lol");
}

headermap_t	HttpRequest::getHeaders() const {
	return (_header);
}

string	HttpRequest::getHost() const {
	return (_host);
}

string	HttpRequest::getPort() const {
	return (_port);
}

bool	HttpRequest::hasBody() const {
	return (_hasBody);
}

size_t	HttpRequest::getBodySize() const {
	return (_bodySize);
}

bool	HttpRequest::hasContentLength() {
	return (_hasContentLength);
}

size_t	HttpRequest::getContentLength() const {
	return (_contentLength);
}

bool	HttpRequest::isMultipart() const {
	return (_isMultipart);
}

bool	HttpRequest::isUrlEncoded() const {
	return (_isUrlEncoded);
}

string	HttpRequest::getMultiType() const {
	return (_multipart.type);
}

string	HttpRequest::getMultiBoundary() const {
	return (_multipart.boundary);
}

size_t	HttpRequest::getMultiPartsNb() const {
	return (_multipart.partNb);
}

vector<string>	HttpRequest::getMultiPartsContents() const {
	return (_multipart.partsContents);
}

vector<headermap_t>	HttpRequest::getMultiPartsHeaders() const {
	return (_multipart.headers);
}

vector<string>				HttpRequest::getMultiPartsFiles() const {
	return (_multipart.partsFiles);
}

string	HttpRequest::getUrlEncodedBody() {
	return (_urlEncodedBody);
}
