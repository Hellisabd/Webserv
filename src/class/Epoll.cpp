#include "Epoll.hpp"

Epoll::Epoll(std::vector<int> sock, int nbr_port) : _sock(sock) {
	_nbr_client = 0;
	_epoll_fd = epoll_create(MAX_EVENTS);
	if (_epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
	for (int i = 0; i != nbr_port; i++) {
		_epollServ.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP | EPOLLOUT;
		_epollServ.data.fd = _sock[i];
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock[i], &_epollServ) == -1)
			throw Error("Error during epoll ctl");
	}
	_epollClient.resize(MAX_EVENTS);
	_ClientSock.resize(MAX_EVENTS);
	for (std::vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
		*i = -1;
}

Epoll::~Epoll() {
	for (std::vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
	{
		if (*i != -1)
			close(*i);
	}
	if (_epoll_fd != -1)
		close(_epoll_fd);
	//debug("passe dans le destructeur de Epoll");
}

void Epoll::wait(int stop) {
	// debug("passe");
	_n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	_time_out = clock();
	if (_n < 0 || stop == 0) {
		close (_epoll_fd);
		for (std::vector<int>::iterator i = _sock.begin(); i != _sock.end(); i++)
			close (*i);
		throw Error("Error during epoll_wait");
	}
}

void Epoll::topars(std::string HTTPRequest, int ClientFD)
{
	static int i = 0;
	i++;
	std::ofstream fd("./request", std::ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug request");
	//debug(PURPLE, "request", i);
	debug_file(HTTPRequest, &fd, ClientFD);
	fd.close();
}

t_requestClient newRequestClient()
{
	t_requestClient _HTTPRequest;
	_HTTPRequest.recvEnd = false;
	_HTTPRequest.sendEnd = false;
	_HTTPRequest.disconnect = false;
	_HTTPRequest.nbr_of_read = 0;
	_HTTPRequest.bodysize = 0;
	return _HTTPRequest;
}

void Epoll::addClient(int port)
{
	int client = accept(_sock[port], NULL, NULL);
	if (client == -1){
		perror("Accept: ");
		throw Error("Failed to accept client connexion");
	}
	int flags = fcntl(client, F_GETFL, 0);
	fcntl(client, F_SETFL, flags | O_NONBLOCK);
	struct epoll_event new_client;
	new_client.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	new_client.data.fd = client;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
		close(client);
		throw Error("Error adding new client to epoll");
	}
	_ClientSock.push_back(client);
	_cliport[client] = _sock[port];
	_HTTPRequest[client] = newRequestClient();
	debug("fd when added :" , client);
}

int validToSend(std::string const &str, clock_t time)
{
	if (clock() - time > 10000)
		return 2;
	if (str.find("\r\n\r\n") != str.npos)
		return 1;
	return 0;
}

std::string getScriptName(std::string url) {
	std::size_t start;
	std::size_t end;
	start = url.find("cgi-bin/", 0);
	if (start == url.npos)
		return "";
	end = url.find("/", start + 8);
	//debug(18);
	if (end == url.npos)
		return "./" + url.substr(start, url.length() - start);
	else
		return "./" + url.substr(start, end - start);
}

void Epoll::set_new_env(Data &data, HttpRequest rq) {
	data._env["PATH_INFO"] = rq.getUrl();
	data._env["SCRIPT_NAME"] = getScriptName(rq.getUrl());
	data._env["REQUEST_METHOD"] = rq.getMethod();
	// if (rq.getMethod() == POST)
	// 	data._env["CONTENT_LENGTH"] = rq.getLength();

}

void Epoll::exec(Data &data, int clientID, HttpRequest rq, std::string req_str)
{
	int fd[2];
	if (pipe(fd) == -1)
		return ;
	std::string text;
	if (req_str.find("text=") != req_str.npos) {
		debug(ORANGE, "find text");
		std::size_t start = req_str.find("text=") + 5;
		if (start != req_str.npos)
			text = req_str.substr(start, req_str.length() - start);
		replace(text);
		data._env["text"] = text;
	}
	int pid = fork();
	if (pid == -1)
	{
		close (fd[0]);
		close (fd[1]);
		return ;
	}
	set_new_env(data, rq);
	if (pid == 0)
	{
		char **env;
		env = data.envToCharpp();
		if (-1 == dup2(fd[1], STDOUT_FILENO))
		{
			close (fd[0]);
			close (fd[1]);
			return ;
		}
		close(fd[0]);
		close(fd[1]);
		char **filename = new char*[2];
		if (rq.getUrl().find("script.php") != rq.getUrl().npos) {
			debug_map(ORANGE, "env", data._env);
			filename[0] = strdup("./script.php");
			filename[1] = NULL;
			execve("cgi-bin/script.php", filename, env);
		}
		else if (rq.getUrl().find("word_count.py") != rq.getUrl().npos) {
			filename[0] = strdup("./word_count.py");
			filename[1] = NULL;
			execve("cgi-bin/word_count.py", filename, env);
		}
		for (int i = 0; env[i]; i++)
			free(env[i]);
		delete[] env;
		free(filename[0]);
		free(filename[1]);
		delete[] filename;
		exit(EXIT_FAILURE);
	}
	waitpid(pid, NULL, 0);
	char buf[20000];
	int byte_read = read(fd[0], buf, sizeof(buf));
	if (byte_read < 0)
	{
		close (fd[0]);
		close (fd[1]);
		return ;
	}
	close(fd[0]);
	close(fd[1]);
	buf[byte_read] = '\0';
	std::ostringstream oss;
	oss << byte_read;
	std::string headerHTTP = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");
	if (send(_epollClient[clientID].data.fd, buf, byte_read, MSG_NOSIGNAL) < 0)
	{
		perror("exec send body");
		throw Error("");
	}
	_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
	_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
	_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
	_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;  // Lire les données du client
	ev.data.fd = _epollClient[clientID].data.fd;  // Descripteur de fichier du client
	_epollClient[clientID].events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	debug(YELLOW, "Passing to  dans exec EPOLLIN fd : ", _epollClient[clientID].data.fd);
	debug(_epollClient[clientID].events);
	epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _epollClient[clientID].data.fd, &ev);
	debug(_epollClient[clientID].events);
}

bool Epoll::checkRequestIsValid(const std::string &url, Data &data, std::string const &method)
{
	std::map<std::string, std::vector<std::string> > tmp =  data.getMethods();
	for (std::map<std::string, std::vector<std::string> >::iterator it = tmp.begin(); it != tmp.end(); ++it)
	{
		if (it->first == url)
		{
			for (std::vector<std::string>::iterator strit = it->second.begin(); strit != it->second.end(); ++strit)
			{
				if (method == *strit)
					return true;
			}
		}
	}
	return false;
}

void	print_in_response(std::string headerHTTP, std::string tosend, int clientFD)
{
	std::string response = headerHTTP + tosend;
	std::ofstream fd("./response", std::ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug response");
	//debug(PURPLE, "request", i);
	debug_file(response, &fd, clientFD);
	fd.close();
}


void Epoll::sendToClient(int clientID, Data &data) {
	std::string page;
	HttpRequest rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
	int valid = validToSend(_HTTPRequest[_epollClient[clientID].data.fd].req, _time_out);
	if (valid == 1)
	{
		topars(_HTTPRequest[_epollClient[clientID].data.fd].req, _epollClient[clientID].data.fd);
		if (!rq.isValid()) {
			page = data.getErrors().find("400")->second;
		}

		rq.parseRequest();
		if (rq.parsingError)
		{
			std::cout << "pourquoi" << endl;
			page = data.getErrors().find("400")->second;
		}
		std::map<string, string> path = rq.getHeaders();
		// debug_map(PURPLE, "map form HttpRequest type", path);
	}
	std::string path = rq.getUrl();
	if (path.find("/upload") != path.npos && rq.getMethodToString() == "POST")
	{
		std::string filename = uploadFile(_HTTPRequest[_epollClient[clientID].data.fd].req);
		std::string tmp_name;
		if (filename.find("/downloads") != filename.npos)
			tmp_name = filename.substr(filename.find("/downloads") + 11, filename.length() - (filename.find("/downloads")) + 11);
		data._uploads.push_back(tmp_name);
		std::vector<std::string> method;
		method.push_back("GET");
		std::map<std::string, std::vector<string> > &tmp = data.getMethods();
		tmp["/downloads/" + tmp_name] = method;
		std::map<std::string, std::string> &tmploc = data.getLocations();
		if (filename.find("/downloads") != filename.npos)
			tmploc["/downloads/" + tmp_name] = filename;
		generate_uploads_url(data._uploads);
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].req.npos != _HTTPRequest[_epollClient[clientID].data.fd].req.find("favicon", 0)){
		_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
		_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
		_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
		_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
		struct epoll_event ev;
		ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;  // Lire les données du client
		ev.data.fd = _epollClient[clientID].data.fd;  // Descripteur de fichier du client
		debug(_epollClient[clientID].events);
		debug(YELLOW, "Passing favicon EPOLLIN fd : ", _epollClient[clientID].data.fd);
		epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _epollClient[clientID].data.fd, &ev);
		debug(_epollClient[clientID].events);
		return ;
	}
	// debug(ORANGE, "req: ", _HTTPRequest[clientID].req);
	//debug(path);
	for(std::map<std::string, std::string>::const_iterator i = data.getLocations().begin(); i != data.getLocations().end() && valid != 2; i++) {
		if (path == i->first)
		{
			page = i->second;
			if (!checkRequestIsValid(i->first, data, rq.getMethodToString()))
			{
				debug("passe dans requete forbiden");
				page = data.getErrors().find("403")->second;
				debug(page);
			}
			break ;
		}
	}
	if (path.find("cgi-bin") != path.npos)
		return exec(data, clientID, rq, _HTTPRequest[_epollClient[clientID].data.fd].req);
	else if (page.empty() && valid == 2)
		page = data.getErrors().find("408")->second;
	else if (page.empty())
		page = data.getErrors().find("404")->second;
	std::ostringstream oss;
	oss << getFileSize(page);
	std::string headerHTTP = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");
	int infile = open(page.c_str(), O_RDONLY);
	if (infile < 0)
	{
		page = data.getErrors().find("403")->second;
	}
	if (page == "./site/submit_comment.html") {
		save_comment(_HTTPRequest[_epollClient[clientID].data.fd].req);
		generate_comment_page();
	}
	char tosend[1024];
	ssize_t file_read;
	while ((file_read = read(infile, tosend, sizeof(tosend))) > 0) {
		if (file_read < 1024)
			tosend[file_read++] = '\0';
		if (send(_epollClient[clientID].data.fd, tosend, file_read, MSG_NOSIGNAL) < 0)
		{
			close(infile);
			perror("client send body");
			throw Error("");
		}
	}
	print_in_response(headerHTTP, tosend, _epollClient[clientID].data.fd);
	_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
	_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
	// debug("passe bool to false");
	_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
	_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;  // Lire les données du client
	ev.data.fd = _epollClient[clientID].data.fd;  // Descripteur de fichier du client
	debug(_epollClient[clientID].events);
	debug(YELLOW, "Passing to EPOLLIN fd in send to client trad path: ", _epollClient[clientID].data.fd);
	epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _epollClient[clientID].data.fd, &ev);
	debug(_epollClient[clientID].events);
	close(infile);
}

std::size_t getbodysize(std::string str, size_t start)
{
	return std::atoi(str.c_str() + start);
}

void Epoll::readFromClient(int clientID)
{
	char buffer[1025];
	ssize_t bytes_read = 0;
	bytes_read = read(_epollClient[clientID].data.fd, buffer, 1024);
	debug("passe bytres_read: ", bytes_read);
	debug("_epollClient[clientID].data.fd: ", _epollClient[clientID].data.fd);
	if (bytes_read < 0)
		return ;
	buffer[bytes_read] = '\0';
		if (bytes_read > 0) {
			_HTTPRequest[_epollClient[clientID].data.fd].req.append(buffer, bytes_read);
			_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read++;
			// debug(bytes_read);
			// debug(_HTTPRequest[clientID].req);
		}
		size_t header_end = _HTTPRequest[_epollClient[clientID].data.fd].req.find("\r\n\r\n");
		if (header_end != std::string::npos)
		{
			size_t content_length_pos = _HTTPRequest[_epollClient[clientID].data.fd].req.find("Content-Length:");
			if (content_length_pos != std::string::npos)
			{
				size_t start = content_length_pos + 15;
				_HTTPRequest[_epollClient[clientID].data.fd].bodysize = getbodysize(_HTTPRequest[_epollClient[clientID].data.fd].req, start);
				if (bytes_read < 1024)
				{
					debug("cense passer sur une requet POST");
					_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = true;
					struct epoll_event ev;
					ev.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP;  // Lire les données du client
					ev.data.fd = _epollClient[clientID].data.fd;  // Descripteur de fichier du client
					debug(_epollClient[clientID].events);
					debug(YELLOW, "Passing to EPOLLOUT fd : ", _epollClient[clientID].data.fd);
					epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _epollClient[clientID].data.fd, &ev);
					debug(_epollClient[clientID].events);
					return ;
				}
			}
			else
			{
				debug("cense passer sur une requet GET");
				_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = true;
				struct epoll_event ev;
				ev.events = EPOLLOUT | EPOLLRDHUP | EPOLLHUP;  // Lire les données du client
				ev.data.fd = _epollClient[clientID].data.fd;  // Descripteur de fichier du client
				debug(_epollClient[clientID].events);
				debug(YELLOW, "Passing to EPOLLOUT fd : ",  _epollClient[clientID].data.fd);
				epoll_ctl(_epoll_fd, EPOLL_CTL_MOD, _epollClient[clientID].data.fd, &ev);
				debug(_epollClient[clientID].events);
				return ;
			}
		}
		else if (bytes_read == 0)
		{
			close(_epollClient[clientID].data.fd);
		}
		// else if (bytes_read == 0 && _HTTPRequest[clientID].nbr_of_read == 0) {
		// 	debug(YELLOW, "disconnect");
		// 	close(_epollClient[clientID].data.fd);
		// 	_HTTPRequest[clientID].disconnect = true;
		// 	return;
		// }
		// debug(buffer);
		// debug(BLUE, "nbr of read: ", _HTTPRequest[clientID].nbr_of_read);
		
}

std::map<int, int>::iterator Epoll::deleteClient(std::map<int, int>::iterator it) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, &_epollServ);
	for (std::vector<int>::iterator fd = _ClientSock.begin(); fd != _ClientSock.end(); ++fd)
	{
		if (*fd == it->first) {
			_ClientSock.erase(fd);
			debug(RED, "Client disconnected.");
			break;
		}
	}
	_noclient = true;
	close(it->first);
	std::map<int, int>::iterator next_it = it;
	++next_it;
	_cliport.erase(it);
	return next_it;
}

bool Epoll::isSockPort(int fd)
{
	for (std::vector<int>::iterator it = _sock.begin(); it != _sock.end(); ++it)
	{
		if (fd == *it)
			return true;
	}
	return false;
}

void Epoll::handleRequest(std::vector<struct sockaddr_in> address, Data &data) {
	std::map<int, int>::iterator it = _cliport.begin();
	_noclient = false;
	static int stat = 0;
	stat++;
	// debug(ORANGE, _n);
	for (int clientID = 0; clientID < _n; clientID++) {
		for (size_t port = 0; port < _sock.size(); port++) {
			if (it == _cliport.end() && (_cliport.size() != 0 || _noclient))
			{
				_noclient = false;
				break;
			}
			// debug("fd: ", _epollClient[clientID].data.fd);
			// debug("event: ", _epollClient[clientID].events);
			debug("it->second :", it->second);
			debug("_sock[port] ", _sock[port]);
			if (_epollClient[clientID].data.fd == _sock[port])
			{
				addClient(port);
				std::ostringstream oss;
				oss << ntohs(address[port].sin_port);
				debug(GREEN, "New client added on port " + oss.str());
			}
			else if (it->second == _sock[port] && !isSockPort(_epollClient[clientID].data.fd)){
				if (_epollClient[clientID].events & (EPOLLHUP | EPOLLRDHUP))
				{
					it = deleteClient(it);
					debug(GREEN, "passe");
					break;
				}
				else if (_HTTPRequest[_epollClient[clientID].data.fd].recvEnd == false && _epollClient[clientID].events & EPOLLIN)
				{
					debug(PURPLE, "EPOLLIN");
					readFromClient(clientID);
					break;
				}
				// debug(it->second);
				// debug(BLUE, _HTTPRequest[_epollClient[clientID].data.fd].recvEnd);
				else if (_HTTPRequest[_epollClient[clientID].data.fd].recvEnd == true && _epollClient[clientID].events & EPOLLOUT)
				{
					debug(PURPLE, "EPOLLOUT");
					sendToClient(clientID, data);
					break ;
				}
				// debug("end of request: ", _HTTPRequest[clientID].recvEnd);
			}
			if (_noclient == true)
			{
				debug("breaking");
				_noclient = false;
				break;
			}
		}
		if (it == _cliport.end())
			break;
		++it;
	}
}
