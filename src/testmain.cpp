#include <iostream>
#include "HttpRequest.hpp"

int main(void) {
	// std::string dummyRq =
	// 	"POST / HTTP/1.1\r\n"
	// 	"Host: 127.0.0.1:4444\r\n"
	// 	"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:131.0) Gecko/20100101 Firefox/131.0\r\n"
	// 	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/png,image/svg+xml,*/*;q=0.8\r\n"
	// 	"Accept-Language: en-US,en;q=0.5\r\n"
	// 	"Accept-Encoding: gzip, deflate, br, zstd\r\n"
	// 	"Connection: keep-alive\r\n"
	// 	"Upgrade-Insecure-Requests: 1\r\n"
	// 	"Sec-Fetch-Dest: document\r\n"
	// 	"Sec-Fetch-Mode: navigate\r\n"
	// 	"Sec-Fetch-Site: none\r\n"
	// 	"Sec-Fetch-User: ?1\r\n"
	// 	"Priority: u=0, i\r\n"
	// 	"\r\n";
	std::string dummyRq =
		"GET / HTTP/1.1\r\nHa: lol \r\n"
		"Host: 127.0.0.1:4444\r\nContent-Length: 1\r\n\r\nhaha";
	HttpRequest rq(dummyRq);
	if (!rq.isValid()) {
		cout << "parsingStrError: " << rq.parsingStrError << endl;
		cout << "400: bad request isvalid";
		exit(1);
	}
	rq.parseRequest();
	if (rq.parsingError) {
		cout << "parsingStrError: " << rq.parsingStrError << endl;
		cout << "400: bad request parsingerror";
		exit(1);
	}
	cout << "host: |" << rq.getHost() << "|" <<  endl
		 << "port: |" << rq.getPort() << "|" << endl
		 << "method: |" << rq.getMethodToString() << "|" << endl
		 << "url: |" << rq.getUrl() << "|" << endl
	     << "httpversion: |" << rq.getHttpVersion() << "|" << endl;
}
