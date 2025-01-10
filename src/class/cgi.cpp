#include "cgi.hpp"
#include <ctime>

cgi::cgi(string script, Data &data, HttpRequest &requestinfo, string request, string &_response, bool &inCgi, clock_t &time, int &pid) {
	int fdrecv[2];
	int fdsend[2];
	if (inCgi == false) {
		time = clock();
		if (pipe(fdsend) == -1)
			return ;
		if (pipe(fdrecv) == -1) {
			close (fdrecv[0]);
			close (fdrecv[1]);
			return ;
		}
		pid = fork();
		if (pid == -1) {
			close (fdrecv[0]);
			close (fdrecv[1]);
			return ;
		}
		if (pid != 0) {
			if (write(fdrecv[1], request.c_str(), request.length()) <= 0)
				throw Disconnect("Error writing."); 
			close(fdrecv[1]);
		}
		if (pid == 0) {
			_argv = get_argv(script);
			set_new_env(data, requestinfo, request);
			_env = data.envToCharpp();
			if (-1 == dup2(fdrecv[0], STDIN_FILENO)) {
				close (fdrecv[0]);
				close (fdrecv[1]);
				close (fdsend[0]);
				close (fdsend[1]);
				return ;
			}
			if (-1 == dup2(fdsend[1], STDOUT_FILENO)) {
				close (fdrecv[0]);
				close (fdrecv[1]);
				close (fdsend[0]);
				close (fdsend[1]);
				return ;
			}
			close(fdrecv[0]);
			close(fdrecv[1]);
			execve(_argv[0], _argv, _env);
			exit(EXIT_FAILURE);
		}
	}
	inCgi = true;
	int result = 0;
	result = waitpid(pid, NULL, WNOHANG);
	if (result > 0)
		inCgi = false;
	if (check_timeout(time, requestinfo.getUrl())) {
		result = 2;
		inCgi = false;
	}
	char buf[20000];
	if (result == 2) {
		kill(pid, SIGTERM);
		int infile = open(data.getErrors().find("500")->second.c_str(), O_RDONLY);
		int byte_read = read(infile, buf, sizeof(buf));
		if (byte_read < 0) {
			close (fdrecv[0]);
			close (fdrecv[1]);
			close (fdsend[0]);
			close (fdsend[1]);
			return ;
		}
		close (fdrecv[0]);
		close (fdrecv[1]);
		close (fdsend[0]);
		close (fdsend[1]);
		buf[byte_read] = '\0';
		ostringstream oss;
		oss << byte_read;
		string headerHTTP = "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\nConnection: close\r\n\r\n";
		_response = (headerHTTP + (string)buf);
		return ;
	}
	if (result > 0) {
		int byte_read = read(fdsend[0], buf, sizeof(buf));
		if (byte_read < 0) {
			close (fdrecv[0]);
			close (fdrecv[1]);
			close (fdsend[0]);
			close (fdsend[1]);
			return ;
		}
		close(fdrecv[0]);
		close(fdrecv[1]);
		close (fdsend[0]);
		close (fdsend[1]);
		buf[byte_read] = '\0';
		ostringstream oss;
		oss << byte_read;
		string headerHTTP = "HTTP/1.1 200 OK\r\nPath=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
		_response = (headerHTTP + (string)buf);
	}
	return ;
}

char **cgi::get_argv(string script) {
	_argv = new char*[2];
	_argv[0] = strdup(("." + script).c_str());
	_argv[1] = NULL;
	return _argv;
}

string findBoundary(string rq) {
	size_t start = rq.find("boundary=") + 9;
	size_t end = rq.find("\r\n", start);
	string boundary = rq.substr(start, end - start);
	return boundary;
}

void cgi::set_new_env(Data &data, HttpRequest &rq, string request) {
	data._env["PATH_INFO"] = rq.getUrl();
	data._env["REQUEST_METHOD"] = rq.getMethod();
	data._env["BOUNDARY"] = findBoundary(request);
}