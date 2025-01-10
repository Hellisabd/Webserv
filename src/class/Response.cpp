#include "Response.hpp"

Response::Response(string status, string &response) : _completedResponse(response) {}

void Response::validResponse(int status) {
	response
}

void Response::StatusHandling() {
	int StatusCode = atoi(_status.c_str());
	switch (StatusCode) {
		case 200:
			validResponse(200);
		case 202:
			validResponse(202);
		case 204:
			validResponse(204);
		case 301:
		case 302:
		case 400:
		case 403:
		case 404:
		case 405:
		case 408:
		case 411:
		case 413:
		case 414:
		case 415:
		case 429:
		case 431:
		case 451:
		case 500:
		case 501:
	}
}