#include "Epoll.hpp"

Epoll::Epoll(std::vector<int> sock, int nbr_port) : _sock(sock) {
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
}

Epoll::~Epoll() {
	for (int i = 0; i < _nbr_client; i++)
		close(_ClientSock[i]);
	if (_epoll_fd != -1)
		close(_epoll_fd);
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
	std::ofstream fd("./out", std::ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug request");
	debug(PURPLE, "request", i);
	debug_file(HTTPRequest, &fd);
	fd.close();
}

std::string quickgetpars(std::string HTTPRequest)
{
	std::string path;
	std::size_t path_start;
	std::size_t path_end;

	path_start = HTTPRequest.find("/");
	path_end = HTTPRequest.find(" ", path_start);
	path = HTTPRequest.substr(path_start, path_end - path_start);

	return path;
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
	_cliport[client] = _sock[port];
	_ClientSock.push_back(client);
}

int validToSend(std::string const &str, clock_t time)
{
	if (clock() - time > 10000)
		return 2;
	if (str.find("\r\n\r\n") != str.npos)
		return 1;
	return 0;
}

void Epoll::exec(Data &data)
{
	int pid = fork();
	if (pid == 0)
	{
		char **env = data.envToCharpp();
		char **filename = new char*[2];
		filename[0] = strdup("./script.php");
		filename[1] = NULL;
		execve("cgi-bin/script.php", filename, env);
		for (int i = 0; env[i]; i++)
			free(env[i]);
		delete[] env;
		free(filename[0]);
		free(filename[1]);
		delete[] filename;
		exit(EXIT_FAILURE);
	}
	waitpid(pid, NULL, 0);
}

void Epoll::sendToClient(int clientID, Data &data) {
	std::string page;
	HttpRequest rq(_HTTPRequest[clientID]);
	int valid = validToSend(_HTTPRequest[clientID], _time_out);
	if (valid == 1)
	{
		topars(_HTTPRequest[clientID]);
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
		debug_map(PURPLE, "map form HttpRequest type", path);
	}
	std::string path = rq.getUrl();
	// debug(BLUE, path);
	if (_HTTPRequest[clientID].npos != _HTTPRequest[clientID].find("favicon", 0)){
		_HTTPRequest[clientID].clear();
		return ;
	}
	debug(path);
	if (path.find("cgi-bin/script.php") != path.npos)
		exec(data);
	for(std::map<std::string, std::string>::const_iterator i = data.getLocations().begin(); i != data.getLocations().end() && valid != 2; i++) {
		if (path == i->first)
		{
			page = i->second;
			break ;
		}
	}
	 if (page.empty() && valid == 2)
		page = data.getErrors().find("408")->second;
	else if (page.empty())
		page = data.getErrors().find("404")->second;
	// debug (page);
	std::string headerHTTP = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: " + std::to_string(getFileSize(page)) + "\r\n\r\n";
	if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");

	int infile = open(page.c_str(), O_RDONLY);
	if (infile < 0)
	{
		page = data.getErrors().find("403")->second;
	}
	char tosend[1024];
	ssize_t file_read;
	while ((file_read = read(infile, tosend, sizeof(tosend))) > 0) {
		if (file_read < 1024)
			tosend[file_read++] = '\0';
		if (send(_epollClient[clientID].data.fd, tosend, file_read, MSG_NOSIGNAL) < 0)
		{
			close(infile);
			perror("send body");
			throw Error("");
		}
	}
	_HTTPRequest[clientID].clear();
	close(infile);
}

void Epoll::readFromClient(int clientID)
{
	char buffer[1024];
	ssize_t bytes_read = 1;
	while (bytes_read > 0) {
		bytes_read = read(_epollClient[clientID].data.fd, buffer, sizeof(buffer));
		if (bytes_read < 0)
			break;
		if (bytes_read < 1024)
			buffer[bytes_read] = '\0';
		_HTTPRequest[clientID] += buffer;
		if (bytes_read < 1024)
			break;
	}
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
	return (_cliport.erase(it));
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
				debug(GREEN, "New client added on port " + std::to_string(ntohs(address[port].sin_port)));
			}
			else if (it->second == _sock[port]) {
				
				readFromClient(clientID);
				if (_HTTPRequest[clientID].length() == 0)
					it = deleteClient(it);
				else
					sendToClient(clientID, data);
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
