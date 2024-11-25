#pragma once

#include <map>
#include <string>
#include <utility>
#include <iostream>
#include "../../includes/webserv.hpp"

using namespace std;
enum HttpMethod {GET, POST, DELETE, UNKNOWN};

typedef map<string, string> strmap_t;

typedef struct s_headerValue {
	string				rawValue;
	strmap_t			parameters;
}	t_headerValue;

typedef map<string, t_headerValue> headermap_t;
typedef pair<string, t_headerValue> headerpair_t;


typedef struct s_multipart {
	string							type;
	string							boundary;
	size_t							partNb;
	vector<string>					partsContents;
	vector<headermap_t>				headers;
	vector<size_t>					headerLens;
	vector<string>					partsFiles;
}	t_multipart;

// Usage : request in the constructor,
// .isValid() first,
// then parseRequest
// then check .parsingError;
class HttpRequest {
 public:
	HttpRequest(string request);
	~HttpRequest();
	bool						isValid();
	bool						isValidRequestLine();
	bool						isValidHost();
	bool						parseRequest();
	string						getMethodToString() const;
	HttpMethod					getMethod() const;
	string						getUrl() const;
	string						getHttpVersion() const;
	string						getBody() const;
	headermap_t					getHeaders() const;
	string						getSpecHeader(string& spec) const;
	size_t						getSize() const;
	string						getHost() const;
	string						getPort() const;
	bool						hasBody() const;
	bool						hasContentLength();
	size_t						getBodySize() const;
	size_t						getContentLength() const;
	bool						isMultipart() const;
	string						getMultiType() const;
	string						getMultiBoundary() const;
	size_t						getMultiPartsNb() const;
	vector<string>				getMultiPartsContents() const;
	vector<headermap_t>			getMultiPartsHeaders() const;
	vector<string>				getMultiPartsFiles() const;
	const pair< const pair<string, t_headerValue> ,bool> getHeaderByKey(const string& key);
	bool	hasParameterKey(const string& paramKey, const strmap_t params);
	const string getParameterValue(const string& paramKey, const strmap_t params);
	bool						isUrlEncoded() const;
	string						getUrlEncodedBody();
	bool						parseHeader();
	bool						parseBody();
	bool						parseAll();

	bool						parsingError;
	string						parsingStrError;
	int							errNo;
 private:
	static const size_t _minRequestSize = 18; // GET / HTTP/1.1\r\n\r\n
	void						setErr(int n, const string& s);
	bool						fillMethod();
	void						fillUrl();
	void						fillHttpVersion();
	bool						fillHeaders();
	bool						fillBody();
	void						fillSize();
	void						fillHostAndPort();
	bool						isEnd(const string::iterator& it);
	bool						isChunkedBasedRequest();
	bool						isValidBoundary();
	bool	allBoundaryAreValid();
	bool isMultipartHeaderRightfullyFormatted(const string& s);

	//utils
	string			extractHeaderKey(std::string &s);
	t_headerValue	extractHeaderValue(string::iterator& it);
	bool			validateHeaderKey(std::string& headerKey);
	void			calcBodySize();
	bool 			extractMultiparts();
	bool			extractMultipartHeaders();
	bool			validateMultipartHeaderKey(string &headerKey, headermap_t hm);
	bool 			extractMultipartFiles();
	bool			validContentType();

	string						_request;
	size_t						_requestLineSize;
	HttpMethod					_method; // get post delete unknown
	string						_host;
	string						_port;
	string						_url;
	string						_httpVersion;
	headermap_t					_header;
	size_t						_requestSize;
	string						_delimiter;
	string::iterator			_headerEnd;
	bool						_hasBody;
	size_t						_bodySize;
	bool						_hasContentLength;
	size_t						_contentLength;
	bool						_isMultipart;
	t_multipart					_multipart;
	bool						_isUrlEncoded;
	string						_urlEncodedBody;
};
