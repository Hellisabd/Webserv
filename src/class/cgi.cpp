#include "cgi.hpp"

cgi::cgi(string script, Data &data, string &_response, t_requestClient &stru, HttpRequest &requestinfo, string &_status) {
	if (stru.cgi == false) {
		string s = "." + script;
		if (access(s.c_str(), F_OK) < 0 || access(s.c_str(), X_OK) < 0) {
			_status = "404";
			return ;
		}
		stru.time = clock();
		if (pipe(stru.fdsend) == -1)
			return ;
		if (pipe(stru.fdrecv) == -1) {
			close (stru.fdrecv[0]);
			close (stru.fdrecv[1]);
			return ;
		}
		stru.pid = fork();
		if (stru.pid == -1) {
			close (stru.fdrecv[0]);
			close (stru.fdrecv[1]);
			return ;
		}
		if (stru.pid != 0) {
			if (write(stru.fdrecv[1], stru.req.c_str(), stru.req.length()) <= 0)
				throw Disconnect("Error writing."); 
			close(stru.fdrecv[1]);
		}
		if (stru.pid == 0) {
			_argv = get_argv(script);
			set_new_env(data, requestinfo, stru.req);
			_env = data.envToCharpp();
			if (-1 == dup2(stru.fdrecv[0], STDIN_FILENO)) {
				close (stru.fdrecv[0]);
				close (stru.fdrecv[1]);
				close (stru.fdsend[0]);
				close (stru.fdsend[1]);
				return ;
			}
			if (-1 == dup2(stru.fdsend[1], STDOUT_FILENO)) {
				close (stru.fdrecv[0]);
				close (stru.fdrecv[1]);
				close (stru.fdsend[0]);
				close (stru.fdsend[1]);
				return ;
			}
			close(stru.fdrecv[0]);
			close(stru.fdrecv[1]);
			close(stru.fdsend[0]);
			close(stru.fdsend[1]);
			execve(_argv[0], _argv, _env);
			free(_argv[0]);
			free(_argv[1]);
			exit(EXIT_FAILURE);
		}
	}
	stru.cgi = true;
	int result = 0;
	int status = 0;
	result = waitpid(stru.pid, &status, WNOHANG);
	if (status > 0) {
		kill(stru.pid, SIGTERM);
		_status = "500";
		close (stru.fdrecv[0]);
		close (stru.fdrecv[1]);
		close (stru.fdsend[0]);
		close (stru.fdsend[1]);
		return ;
	}

	if (result > 0)
		stru.cgi = false;
	if (check_timeout(stru.time)) {
		result = 2;
		stru.cgi = false;
	}
	if (result == 2) {
		kill(stru.pid, SIGTERM);
		_status = "500";
		close (stru.fdrecv[0]);
		close (stru.fdrecv[1]);
		close (stru.fdsend[0]);
		close (stru.fdsend[1]);
		return ;
	}
	if (result > 0) {
		char buf[20000];
		int byte_read = read(stru.fdsend[0], buf, sizeof(buf));
		if (byte_read < 0) {
			close (stru.fdrecv[0]);
			close (stru.fdrecv[1]);
			close (stru.fdsend[0]);
			close (stru.fdsend[1]);
			return ;
		}
		close(stru.fdrecv[0]);
		close(stru.fdrecv[1]);
		close(stru.fdsend[0]);
		close(stru.fdsend[1]);
		buf[byte_read] = '\0';
		_response = (string)buf;
		if (requestinfo.getUrl() == "/cgi-bin/add_file.py")
			_status = "202";
		else
			_status = "200";
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