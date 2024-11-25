#include "Epoll.hpp"

Epoll::Epoll(std::vector<int> sock, int nbr_port) : _sock(sock) {
	_HTTPRequest = new t_requestClient[MAX_EVENTS];
	_nbr_client = 0;
	_epoll_fd = epoll_create(MAX_EVENTS);
	if (_epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
	for (int i = 0; i != nbr_port; i++) {
		_epollServ.events = EPOLLIN;
		_epollServ.data.fd = _sock[i];
		if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock[i], &_epollServ) == -1)
			throw Error("Error during epoll ctl");
	}
	_epollClient.resize(MAX_EVENTS);
	_ClientSock.resize(MAX_EVENTS);
	for (std::vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
		*i = -1;
	for (int j = 0; j != MAX_EVENTS; j++) {
		_HTTPRequest[j].recvEnd = false;
		_HTTPRequest[j].sendEnd = false;
		_HTTPRequest[j].disconnect = false;
		_HTTPRequest[j].nbr_of_read = 0;
		_HTTPRequest[j].bodysize = 0;
	}
}

Epoll::~Epoll() {
	for (std::vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
	{
		if (*i != -1)
			close(*i);
	}
	if (_epoll_fd != -1)
		close(_epoll_fd);
	delete[] _HTTPRequest;
	//debug("passe dans le destructeur de Epoll");
}

void Epoll::wait(int stop) {
	_n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	_time_out = clock();
	if (_n < 0 || stop == 0) {
		close (_epoll_fd);
		for (std::vector<int>::iterator i = _sock.begin(); i != _sock.end(); i++)
			close (*i);
		throw Error("Error during epoll_wait");
	}
}

void topars(std::string HTTPRequest)
{
	static int i = 0;
	i++;
	std::ofstream fd("./request", std::ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug request");
	//debug(PURPLE, "request", i);
	debug_file(HTTPRequest, &fd);
	fd.close();
}

void Epoll::addClient(int port)
{
	int client = accept(_sock[port], NULL, NULL);
	if (client == -1){
		perror("Accept: ");
		throw Error("Failed to accept client connexion");
	}
	struct epoll_event new_client;
	new_client.events = EPOLLIN;
	new_client.data.fd = client;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
		close(client);
		throw Error("Error adding new client to epoll");
	}
	_ClientSock.push_back(client);
	_cliport[client] = _sock[port];
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
	_HTTPRequest[clientID].req.clear();
	_HTTPRequest[clientID].body.clear();
	_HTTPRequest[clientID].recvEnd = false;
	_HTTPRequest[clientID].nbr_of_read = 0;
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

void	print_in_response(std::string headerHTTP, std::string tosend)
{
	std::string response = headerHTTP + tosend;
	std::ofstream fd("./response", std::ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug response");
	//debug(PURPLE, "request", i);
	debug_file(response, &fd);
	fd.close();
}


void Epoll::sendToClient(int clientID, Data &data) {
	std::string page;
	HttpRequest rq(_HTTPRequest[clientID].req);
	int valid = validToSend(_HTTPRequest[clientID].req, _time_out);
	if (valid == 1)
	{
		topars(_HTTPRequest[clientID].req);
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
		std::string filename = uploadFile(_HTTPRequest[clientID].req);
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
	if (_HTTPRequest[clientID].req.npos != _HTTPRequest[clientID].req.find("favicon", 0)){
		_HTTPRequest[clientID].req.clear();
		_HTTPRequest[clientID].recvEnd = false;
		_HTTPRequest[clientID].nbr_of_read = 0;
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
		return exec(data, clientID, rq, _HTTPRequest[clientID].req);
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
		save_comment(_HTTPRequest[clientID].req);
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
	print_in_response(headerHTTP, tosend);
	_HTTPRequest[clientID].req.clear();
	_HTTPRequest[clientID].nbr_of_read = 0;
	// debug("passe bool to false");
	_HTTPRequest[clientID].recvEnd = false;
	close(infile);
}

std::size_t getbodysize(std::string str)
{
	return std::atoi(str.c_str() + (str.find("Content-Length") + 16));
}

void Epoll::readFromClient(int clientID)
{
	char buffer[1025];
	ssize_t bytes_read = 0;
	bytes_read = read(_epollClient[clientID].data.fd, buffer, sizeof(buffer) - 1);
	buffer[bytes_read] = '\0';
	if (bytes_read > 0 && !_HTTPRequest[clientID].bodysize) {
		_HTTPRequest[clientID].req += buffer;
		_HTTPRequest[clientID].nbr_of_read++;
		// debug("add to request");
	}
	if (_HTTPRequest[clientID].req.find("\r\n\r\n") != std::string::npos && _HTTPRequest[clientID].req.find("Content-Length") != std::string::npos && _HTTPRequest[clientID].body.length() == 0)
	{
		_HTTPRequest[clientID].bodysize = getbodysize(_HTTPRequest[clientID].req);
		// debug("set body size at : ", _HTTPRequest[clientID].bodysize);
		_HTTPRequest[clientID].body = _HTTPRequest[clientID].req.substr(_HTTPRequest[clientID].req.find("\r\n\r\n"), _HTTPRequest[clientID].req.length() -  _HTTPRequest[clientID].req.find("\r\n\r\n"));
		_HTTPRequest[clientID].req = _HTTPRequest[clientID].req.substr(0, _HTTPRequest[clientID].req.find("\r\n\r\n"));
		// _HTTPRequest[clientID].nbr_of_read++;
		// debug(_HTTPRequest[clientID].body);
	}
	else if (_HTTPRequest[clientID].body.length() < _HTTPRequest[clientID].bodysize)
	{
		// debug("add to buffer");
		_HTTPRequest[clientID].body += buffer;
		_HTTPRequest[clientID].nbr_of_read++;
	}
	else if (_HTTPRequest[clientID].req.find("\r\n\r\n") != std::string::npos && !_HTTPRequest[clientID].bodysize)
	{
		_HTTPRequest[clientID].nbr_of_read = 0;
		_HTTPRequest[clientID].recvEnd = true;
	}
	if (_HTTPRequest[clientID].bodysize && _HTTPRequest[clientID].body.length() >= _HTTPRequest[clientID].bodysize)
	{
		_HTTPRequest[clientID].nbr_of_read = 0;
		_HTTPRequest[clientID].recvEnd = true;
		_HTTPRequest[clientID].req += _HTTPRequest[clientID].body;
		// debug("passe dans end of request");
		// debug(_HTTPRequest[clientID].body);
		// debug("body size : ", _HTTPRequest[clientID].bodysize);
		// debug("body length : ", _HTTPRequest[clientID].body.length());
	}
	// debug(_HTTPRequest[clientID].body);
	// debug(buffer);
	// debug(BLUE, "nbr of read: ", _HTTPRequest[clientID].nbr_of_read);
	// if (_HTTPRequest[clientID].req.find("\r\n\r\n") )
	// 	break;
	if (bytes_read == 0 && _HTTPRequest[clientID].nbr_of_read == 0) {
		// debug(YELLOW, "disconnect");
		close(_epollClient[clientID].data.fd);
		_HTTPRequest[clientID].disconnect = true;
		return;
	}
	// _HTTPRequest[clientID].nbr_of_read++;
// }
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

void Epoll::handleRequest(std::vector<struct sockaddr_in> address, Data &data) {
	std::map<int, int>::iterator it = _cliport.begin();
	_noclient = false;
	for (int clientID = 0; clientID < _n; clientID++) {
		for (size_t port = 0; port < _sock.size(); port++) {
			if (it == _cliport.end() && (_cliport.size() != 0 || _noclient))
			{
				_noclient = false;
				break;
			}
			if (_epollClient[clientID].data.fd == _sock[port])
			{
				addClient(port);
				std::ostringstream oss;
				oss << ntohs(address[port].sin_port);
				debug(GREEN, "New client added on port " + oss.str());
			}
			else if (it->second == _sock[port]) {
				// debug(PURPLE, clientID);

				if (_HTTPRequest[clientID].recvEnd == false)
					readFromClient(clientID);
				if (_HTTPRequest[clientID].disconnect == true) {
					_HTTPRequest[clientID].disconnect = false;
					it = deleteClient(it);
				}
				else if (_HTTPRequest[clientID].recvEnd == true)
					sendToClient(clientID, data);
				// debug("end of request: ", _HTTPRequest[clientID].recvEnd);
			}
			if (_noclient == true)
			{
				_noclient = false;
				break;
			}
		}
		if (it == _cliport.end())
			break;
		++it;
	}
}
