#include "cgi.hpp"

// cgi::~cgi() {
// 	for (int i = 0; _env[i]; i++)
// 		free(_env[i]);
// 	delete[] _env;
// 	free(_argv[0]);
// 	free(_argv[1]);
// 	delete[] _argv;	
// }

cgi::cgi(string script, Data &data, int fd_cli) {
	int fd[2];
	if (pipe(fd) == -1)
		return ;
	int pid = fork();
	if (pid == -1) {
		close (fd[0]);
		close (fd[1]);
		return ;
	}
	if (pid == 0) {
		_env = data.envToCharpp();
		_argv = get_argv(script);
		if (-1 == dup2(fd[1], STDOUT_FILENO)) {
			close (fd[0]);
			close (fd[1]);
			return ;
		}
		close(fd[0]);
		close(fd[1]);
		execve(_argv[0], _argv, _env);
		exit(EXIT_FAILURE);
	}
	waitpid(pid, NULL, 0);
	char buf[20000];
	int byte_read = read(fd[0], buf, sizeof(buf));
	if (byte_read < 0) {
		close (fd[0]);
		close (fd[1]);
		return ;
	}
	close(fd[0]);
	close(fd[1]);
	buf[byte_read] = '\0';
	ostringstream oss;
	oss << byte_read;
	string headerHTTP = "HTTP/1.1 200 OK\r\n; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	if (send(fd_cli, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");
	if (send(fd_cli, buf, byte_read, MSG_NOSIGNAL) < 0) {
		perror("exec send body");
		throw Error("");
	}
	// _HTTPRequest[fd_cli].req.clear();
	// _HTTPRequest[fd_cli].recvEnd = false;
	// _HTTPRequest[fd_cli].nbr_of_read = 0;
	// _HTTPRequest[fd_cli].size_to_reach = 0;
	// _HTTPRequest[fd_cli].bodysize = 0;
	// modifEvents(fd_cli, EPOLLIN, _epoll_fd);
	return ;
}

char **cgi::get_argv(string script) {
	_argv = new char*[2];
	_argv[0] = strdup(("." + script).c_str());
	_argv[1] = NULL;
	return _argv;
}