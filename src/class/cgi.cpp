#include "cgi.hpp"

// cgi::~cgi() {
// 	for (int i = 0; _env[i]; i++)
// 		free(_env[i]);
// 	delete[] _env;
// 	free(_argv[0]);
// 	free(_argv[1]);
// 	delete[] _argv;	d
// }

cgi::cgi(string script, Data &data, int fd_cli, HttpRequest &requestinfo, string request) {
	int fdrecv[2];
	int fdsend[2];
	if (pipe(fdsend) == -1)
		return ;
	if (pipe(fdrecv) == -1)
	{
		close (fdrecv[0]);
		close (fdrecv[1]);
		return ;
	}
	int pid = fork();
	if (pid == -1) {
		close (fdrecv[0]);
		close (fdrecv[1]);
		return ;
	}
	if (pid != 0)
	{
		write(fdrecv[1], request.c_str(), request.length());
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
	waitpid(pid, NULL, 0);
	char buf[20000];
	int byte_read = read(fdsend[0], buf, sizeof(buf));
	if (byte_read < 0) {
		close (fdrecv[0]);
		close (fdrecv[1]);
		close (fdsend[0]);
		close (fdsend[1]);
		return ;
	}
	close(fdrecv[0]);
	close (fdsend[0]);
	close (fdsend[1]);
	buf[byte_read] = '\0';
	ostringstream oss;
	oss << byte_read;
	string headerHTTP;
	if (send(fd_cli, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");
	if (send(fd_cli, buf, byte_read, MSG_NOSIGNAL) < 0) {
		throw Error("exec send body");
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
	data._env["FILENAME"] = _filename;
}