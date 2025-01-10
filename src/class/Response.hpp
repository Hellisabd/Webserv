#pragma once
#include <webserv.hpp>
#include <ctime>
using namespace std;


class Data;
class HttpRequest;
class Client;
class cgi;

class Response {

	public:

		Response(string status, string &_response);
		~Response() {}
		void StatusHandling();
		void validResponse(int status);

	private:
		string &_completedResponse;
};