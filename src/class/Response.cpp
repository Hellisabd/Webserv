#include "Response.hpp"

Response::Response(string status, string &response, string &id, Data data) : _id(id), _status(status) {
	StatusHandling(response, data);
}

void Response::validResponse(int status, string &response) {
	string temp;
	ostringstream oss;
	oss << response.length();
	switch (status) {
		case 200:
			temp = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 202:
			temp = "HTTP/1.1 202 Created\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 204:
			temp = "HTTP/1.1 204 No Content\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\n\r\n";
	}
	temp += response;
	response = temp;
}

void Response::errorHandling(int status, string &response, Data data)
{
	string str;
	string body;
	ostringstream oss;
	ifstream inputfile(data.getErrors().find(_status)->second.c_str());
	if (!inputfile.is_open())
		throw Disconnect("Error opening Error file");
	while (getline(inputfile, str))
		body += str;
	oss << body.length();
	switch (status) {
		case 400:
			response = "HTTP/1.1 400 Bad Request\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 401:
			response = "HTTP/1.1 401 Unauthorized\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 403:
			response = "HTTP/1.1 403 Forbidden\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 404:
			response = "HTTP/1.1 404 Not Found\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 405:
			response = "HTTP/1.1 405 Method Not Allowed\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 408:
			response = "HTTP/1.1 408 Request Timeout\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 411:
			response = "HTTP/1.1 411 Length Required\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 413:
			response = "HTTP/1.1 413 Payload Too Large\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 414:
			response = "HTTP/1.1 414 URI Too Long\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 415:
			response = "HTTP/1.1 415 Unsupported Media Type\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 429:
			response = "HTTP/1.1 429 Too Many Requests\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 431:
			response = "HTTP/1.1 431 Request Header Fields Too Large\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 451:
			response = "HTTP/1.1 451 Unavailable For Legal Reasons\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 500:
			response = "HTTP/1.1 500 Internal Server Error\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		case 501:
			response = "HTTP/1.1 501 Not Implemented\r\nSet-Cookie: session_id=" + _id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	}
	response += body;
}

void Response::StatusHandling(string &response, Data data) {
	int StatusCode = atoi(_status.c_str());
	switch (StatusCode) {
		case 200:
			validResponse(200, response);
		case 202:
			validResponse(202, response);
		case 204:
			validResponse(204, response);
		default:
			errorHandling(StatusCode, response, data);
	}
}