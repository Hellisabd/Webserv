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

		Response(string status, string &_response, string &id, Data &data);
		~Response() {}
		void StatusHandling(string &response, Data &data);
		void validResponse(int status, string &response);
		void errorHandling(int status, string &response, Data &data);

	private:
		string _id;
		string _status;
};