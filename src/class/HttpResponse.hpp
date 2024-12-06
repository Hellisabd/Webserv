#include <map>
#include <string>
#include <utility>
#include <iostream>
#include "../../includes/webserv.hpp"

using namespace std;

const string crlf("\r\n");
const string doubleCrlf("\r\n\r\n");
const string httpVersion("HTTP/1.1");


class HttpResponse {
 public:
	HttpResponse(const HttpRequest& rq, const Data& data): _rq(&rq) {
		_statusMap = createStatusMap();
	}
	HttpResponse(int statusCode): _rq(NULL) {
		_statusMap = createStatusMap();
	};
	~HttpResponse();
	string				getResponse();
	int					getStatusCode();
	size_t				getResponseLength();

 private:
	map<string, string>	createStatusMap() {
		map<string, string> status;
		status["200"] = "ok";
		status["202"] = "accepted";
		status["204"] = "no content";
		status["301"] = "moved permanently";
		status["302"] = "found";
		status["400"] = "bad request";
		status["401"] = "unauthorized";
		status["403"] = "forbidden";
		status["404"] = "not found";
		status["405"] = "not allowed";
		status["408"] = "timeout";
		status["411"] = "length required";
		status["413"] = "payload too large";
		status["414"] = "uri too long";
		status["415"] = "unsuported media type";
		status["429"] = "too many requests";
		status["431"] = "request header fields too large";
		status["451"] = "unavailable for legal reasons";
		status["500"] = "internal server error";
		status["501"] = "not implemented";
		return (status);
	}
	void				catHeaders();
	void				genStatusLine(int statusCode);

	vector<headermap_t>	_headers;
	map<string, string>	_statusMap;
	int					_statusCode;
	const HttpRequest	*_rq;

	// final outputs
	string				_statusLineFinal;
	string				_headersFinal;
	string 				_bodyFinal;
};
