#include "Epoll.hpp"

Epoll::Epoll(vector<int> sock, int nbr_port) : _sock(sock) {
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
}

Epoll::~Epoll() {
	if (_epoll_fd != -1)
		close(_epoll_fd);
}

void Epoll::wait(int stop) {
	_n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	_time_out = clock();
	if (_n < 0 || stop == 0) {
		close (_epoll_fd);
		for (vector<int>::iterator i = _sock.begin(); i != _sock.end(); i++)
			close (*i);
		throw Error("Error during epoll_wait");
	}
}

void Epoll::topars(string HTTPRequest, int ClientFD) {
	ofstream fd("./request", ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug request");
	debug_file(HTTPRequest, &fd, ClientFD);
	fd.close();
}

t_requestClient newRequestClient() {
	t_requestClient _HTTPRequest;
	_HTTPRequest.recvEnd = false;
	_HTTPRequest.sendEnd = false;
	_HTTPRequest.disconnect = false;
	_HTTPRequest.sending = false;
	_HTTPRequest.uploading = false;
	_HTTPRequest.nbr_of_read = 0;
	_HTTPRequest.size_to_reach = 0;
	_HTTPRequest.bodysize = 0;
	_HTTPRequest.Loged = false;
	_HTTPRequest.cgi = false;
	return _HTTPRequest;
}

void Epoll::addClient(int port) {
	struct sockaddr_in	addr;
	socklen_t			addrLen = sizeof(addr);

	int client = accept(_sock[port], (struct sockaddr *)&addr, &addrLen);
	if (client == -1) {
		perror("Accept: ");
		throw Error("Failed to accept client connexion");
	}
	fcntl(client, F_SETFL, O_NONBLOCK);
	struct epoll_event new_client;
	new_client.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	new_client.data.fd = client;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
		close(client);
		throw Error("Error adding new client to epoll");
	}
	_cliport[client] = _sock[port];
	_HTTPRequest[client] = newRequestClient();
}

int validToSend(string const &str) {
	if (str.find("\r\n\r\n") != str.npos)
		return 1;
	return 0;
}

int check_timeout(clock_t time, string url) {
	(void)url;
	if (clock() - time > 10000000)
		return 2;
	// debug(clock() - time);
	return 0;
}

bool Epoll::checkRequestIsValid(const string &url, Data &data, string const &method) {
	map<string, vector<string> > tmp =  data.getMethods();
	for (map<string, vector<string> >::iterator it = tmp.begin(); it != tmp.end(); ++it) {
		if (it->first == url) {
			for (vector<string>::iterator strit = it->second.begin(); strit != it->second.end(); ++strit) {
				if (method == *strit)
					return true;
			}
		}
	}
	return false;
}

void	print_in_response(string headerHTTP, string tosend, int clientFD) {
	string response = headerHTTP + tosend;
	ofstream fd("./response", ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug response");
	debug_file(response, &fd, clientFD);
	fd.close();
}

string findSessionID(string request) {
	string null = "default";
	size_t i = request.find("session_id=");
	if (i != string::npos) {
		i += 11;
		if(request[i] != '\r' || request[i] != '\n') {
			size_t end = request.find('\r', i);
			null = request.substr(i, end - i);
			return null;
		}
	}
	return null;
}

void clearLogMsg(string page) {
	std::ifstream inputFile(page.c_str());
	if (!inputFile.is_open()) {
		std::cerr << "Error: Unable to open file " << page << std::endl;
		return;
	}
	std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();
	size_t start = content.find("<div style=\"position: absolute; top: 10px; right: 10px;");
	content = content.substr(0, start - 1);
	std::ofstream outputFile(page.c_str(), std::ios::trunc);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open file for writing " << page << std::endl;
		return;
	}
	outputFile << content;
	outputFile.close();
}

void Epoll::addLogMessage(string user, int index) {
	_HTTPRequest[index].logMsg =
	"<div style=\"position: absolute; top: 10px; right: 10px; "
	"background-color: #f0f0f0; padding: 5px 10px; border: 1px solid #ccc; "
	"border-radius: 5px; font-family: Arial, sans-serif;\">\n"
	"    Log as: " + user + "\n<br>"
	"<a href=\"/logout\">logout</a>"
	"</div>\n";
}

string Epoll::findRightUser(string id) {
	for (vector<Client>::iterator it = _ClientsData.begin(); it != _ClientsData.end(); ++it) {
		if ((*it).getID() == id)
			return (*it).getUser();
	}
	string truc;
	return truc;
}

map<int, int>::iterator Epoll::sendToClient(int clientID, Data &data, map<int, int>::iterator it) {
	string page;
	string id;
	HttpRequest rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
	if (_HTTPRequest[_epollClient[clientID].data.fd].sending == false) {
		int valid = validToSend(_HTTPRequest[_epollClient[clientID].data.fd].req);
		if (valid == 1) {
			topars(_HTTPRequest[_epollClient[clientID].data.fd].req, _epollClient[clientID].data.fd);
			rq.parseAll();
			if (rq.parsingError) {
				cout << rq.parsingStrError << endl;
				page = data.getErrors().find("400")->second;
			}
			if (_HTTPRequest[_epollClient[clientID].data.fd].bodysize > data.getMaxBodySize()) {
				page = data.getErrors().find("413")->second; // a gerer;
				// _response = "HTTP/1.1 413 Payload Too Large\r\n"
				// 				"Content-Type: text/html\r\n"
				// 				"Content-Length: 142\r\n"
				// 				"Connection: close\r\n"
				// 				"\r\n";
			}
			_HTTPRequest[_epollClient[clientID].data.fd].connectionType = rq.getHeaderByKey("Connection").first.second.rawValue;
		}
		string path = rq.getUrl();
		for (map<string, string>::const_iterator itm = data.getRedirections().begin(); itm !=  data.getRedirections().end(); ++itm) {
			if (path == itm->first) {
				_response = "HTTP/1.1 302 Moved Temporary\r\n"
								"Location: " + itm->second + "\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n"
								"\r\n";
				return it;
			}
		}
		if (path == "/logout") {
			id = "default";
			path = "/";
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = false;
		}
		valid = check_timeout(_time_out, path);
		if (path.find("/try_login") != path.npos) {
			Client tmp = login(_HTTPRequest[_epollClient[clientID].data.fd].req, path);
			if (!tmp.getUser().empty()) {
				_ClientsData.push_back(tmp);
				id = tmp.getID();
				_HTTPRequest[_epollClient[clientID].data.fd].Loged = true;
			}
		}
		if (rq.getUrl() == "/favicon.ico") {
			_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
			_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
			_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].uploading = false;
			modifEvents(_epollClient[clientID].data.fd, EPOLLIN, _epoll_fd);
			return it;
		}
		for(map<string, string>::const_iterator i = data.getLocations().begin(); i != data.getLocations().end() && valid != 2 && page.empty(); i++) {
			if (path == i->first) {
				page = i->second;
				if (!checkRequestIsValid(i->first, data, rq.getMethodToString())) 
					page = data.getErrors().find("403")->second;
				break ;
			}
		}
		if (path.find("cgi-bin") != path.npos) {
			id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
			cgi execcgi(path, data, rq, _HTTPRequest[_epollClient[clientID].data.fd].req, _response, _HTTPRequest[_epollClient[clientID].data.fd].cgi, _HTTPRequest[_epollClient[clientID].data.fd].time, _HTTPRequest[_epollClient[clientID].data.fd].pid);
			string filename = find_filename(_HTTPRequest[_epollClient[clientID].data.fd].req);
			data.add_upload(filename);
			if (_HTTPRequest[_epollClient[clientID].data.fd].cgi == false)
			{
				_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
				_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
				_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
				_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
				_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
				modifEvents(_epollClient[clientID].data.fd, EPOLLIN, _epoll_fd);
			}
			return it;
		}
		else if (path.find("delete") != path.npos && rq.getMethodToString() == "DELETE")
			delete_file(path, data);
		else if (page.empty() && valid == 2 && _HTTPRequest[_epollClient[clientID].data.fd].uploading == false)
			page = data.getErrors().find("408")->second;
		else if (page.empty())
			page = data.getErrors().find("404")->second;
		if (id.empty())
			id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
		if ((id == "default" && _HTTPRequest[_epollClient[clientID].data.fd].Loged == false) || _ClientsData.empty())
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = false;
		else
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = true;
		ostringstream oss;
		if (rq.getMethodToString() == "POST" || rq.getMethodToString() == "GET") {
			string headerHTTP;
			if (rq.getUrl().find("downloads/") != rq.getUrl().npos && check_file_availability(rq.getUrl(), data) == false) {
				page = data.getErrors().find("404")->second;
				oss << getFileSize(page);
				_HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send = getFileSize(page);
				headerHTTP = "HTTP/1.1 404 Not Found\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
			}
			else {
				if (_HTTPRequest[_epollClient[clientID].data.fd].Loged == true && _HTTPRequest[_epollClient[clientID].data.fd].logMsg.empty()) {
					_HTTPRequest[_epollClient[clientID].data.fd].Loged = true;
					addLogMessage(findRightUser(id), _epollClient[clientID].data.fd);
				}
				if (rq.getUrl() != "/download") {
					oss << getFileSize(page) + _HTTPRequest[_epollClient[clientID].data.fd].logMsg.length();
					_HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send = getFileSize(page);
					headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
				}
				else
					return sending_upload(generate_upload_page(data._uploads), _epollClient[clientID].data.fd, id, it);
			}
			_HTTPRequest[_epollClient[clientID].data.fd].headerresponse = headerHTTP;
			_response = headerHTTP;
		}
		else if (rq.getMethodToString() == "DELETE") {
			string headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
			_HTTPRequest[_epollClient[clientID].data.fd].headerresponse = headerHTTP;
			_response = headerHTTP;
			_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
			_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
			_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
			_HTTPRequest[_epollClient[clientID].data.fd].sending = false;
			_HTTPRequest[_epollClient[clientID].data.fd].uploading = false;
			modifEvents(_epollClient[clientID].data.fd, EPOLLIN, _epoll_fd);
			return it;
		}
		debug("open error page");
		int infile = open(page.c_str(), O_RDONLY);
		_HTTPRequest[_epollClient[clientID].data.fd].infile = infile;
		if (infile < 0)
			page = data.getErrors().find("403")->second;
		_HTTPRequest[_epollClient[clientID].data.fd].page = page;
	}
	if (id.empty())
		id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
	return (sendingFile(_epollClient[clientID].data.fd, _HTTPRequest[_epollClient[clientID].data.fd].infile, _HTTPRequest[_epollClient[clientID].data.fd].headerresponse, _HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send, it));
}

map<int, int>::iterator	Epoll::sending_upload(std::string page, int index, string id, map<int, int>::iterator it) {
	ostringstream oss;
	oss << page.length() + _HTTPRequest[index].logMsg.length();
	string headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	_response = headerHTTP + page + _HTTPRequest[index].logMsg;
	return (it);
}

map<int, int>::iterator	Epoll::sendingFile(int fd, int infile, string headerHTTP, size_t size_to_send, map<int, int>::iterator it) {
	char tosend[1024];
	ssize_t file_read;
	_HTTPRequest[fd].sending = true;
	file_read = read(infile, tosend, sizeof(tosend));
	_HTTPRequest[fd].size_to_reach += file_read;
	if (file_read < 0)
		throw Disconnect("Error reading file.");
	if (file_read < 1024)
		tosend[file_read] = '\0';
	if (!_response.empty())
		_response += tosend;
	else
		_response = tosend;
	if (_HTTPRequest[fd].size_to_reach >= size_to_send) {
		if (_HTTPRequest[fd].Loged == true) {
			_response += _HTTPRequest[fd].logMsg;
		}
		(void)headerHTTP;
		// print_in_response(headerHTTP, tosend, fd);
		_HTTPRequest[fd].req.clear();
		_HTTPRequest[fd].page.clear();
		_HTTPRequest[fd].nbr_of_read = 0;
		_HTTPRequest[fd].recvEnd = false;
		_HTTPRequest[fd].bodysize = 0;
		_HTTPRequest[fd].size_to_reach = 0;
		_HTTPRequest[fd].sending = false;
		_HTTPRequest[fd].uploading = false;
		modifEvents(fd, EPOLLIN, _epoll_fd);
		close(infile);
	}
	return it;
}

size_t getbodysize(string str, size_t start) {
	return atoi(str.c_str() + start);
}

void Epoll::modifEvents(int fd, int event, uint32_t epoll_fd) {
	struct epoll_event ev;
	ev.events = event | EPOLLRDHUP | EPOLLHUP;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

map<int, int>::iterator Epoll::readFromClient(int clientID, map<int, int>::iterator it) {
	char buffer[1025];
	ssize_t bytes_read = 0;
	bytes_read = recv(_epollClient[clientID].data.fd, buffer, 1024, 0);
	if (bytes_read < 0) {
		deleteClient(_epollClient[clientID].data.fd);
		return it;
	}
	buffer[bytes_read] = '\0';
	if (bytes_read > 0) {
		_HTTPRequest[_epollClient[clientID].data.fd].req.append(buffer, bytes_read);
		if (_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read != 0)
			_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read++;
		_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach += bytes_read;
		_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = true;
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read == 0) {
		size_t header_end = _HTTPRequest[_epollClient[clientID].data.fd].req.find("\r\n\r\n");
		_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read++;
		if (header_end != string::npos) {
			_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach -= header_end;
			size_t content_length_pos = _HTTPRequest[_epollClient[clientID].data.fd].req.find("Content-Length:");
			if (content_length_pos != string::npos) {
				size_t start = content_length_pos + 15;
				_HTTPRequest[_epollClient[clientID].data.fd].bodysize = getbodysize(_HTTPRequest[_epollClient[clientID].data.fd].req, start);
				if (bytes_read < 1024) {
					_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
					_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
					modifEvents(_epollClient[clientID].data.fd, EPOLLIN | EPOLLOUT, _epoll_fd);
					return it;
				}
			}
			else {
					_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
					_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
					modifEvents(_epollClient[clientID].data.fd, EPOLLIN | EPOLLOUT, _epoll_fd);
				return it;
			}
		}
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach >= _HTTPRequest[_epollClient[clientID].data.fd].bodysize) {
		_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
		_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
		// debug(_HTTPRequest[_epollClient[clientID].data.fd].req);
		modifEvents(_epollClient[clientID].data.fd,  EPOLLIN | EPOLLOUT, _epoll_fd);
	}
	return it;
}

bool Epoll::isSockPort(int fd) {
	for (vector<int>::iterator it = _sock.begin(); it != _sock.end(); ++it) {
		if (fd == *it)
			return true;
	}
	return false;
}

void Epoll::deleteClient(int fd) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, fd, &_epollServ);
	close(fd);
}

void Epoll::handleRequest(Data &data) {
	map<int, int>::iterator it = _cliport.begin();
	for (int clientID = 0; clientID < _n; clientID++) {
		try {
			for (size_t port = 0; port < _sock.size(); port++) {
				if (_epollClient[clientID].data.fd == _sock[port]) {
					addClient(port);
				}
				else if (it->second == _sock[port] && !isSockPort(_epollClient[clientID].data.fd)) {
					if (_epollClient[clientID].events & (EPOLLHUP | EPOLLRDHUP | EPOLLERR)) {
						deleteClient(_epollClient[clientID].data.fd);
						break;
					}
					else if (_epollClient[clientID].events & EPOLLIN) {
						it = readFromClient(clientID, it);
						break;
					}
					else if (_epollClient[clientID].events & EPOLLOUT && _HTTPRequest[_epollClient[clientID].data.fd].recvEnd == true) {
						it = sendToClient(clientID, data, it);
						sendingToClient(_epollClient[clientID].data.fd);
						break ;
					}
				}
			}
		if (it == _cliport.end())
			break;
		++it;
		}
		catch (Disconnect const &e) {
			deleteClient(_epollClient[clientID].data.fd);
		}
	}
}

void Epoll::sendingToClient(int fd) {
	if (_HTTPRequest[fd].cgi == false)
	{
		if (send(fd, _response.c_str(), _response.length(), MSG_NOSIGNAL) <= 0)
			throw Disconnect("");
	}
}