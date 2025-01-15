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

void Epoll::wait() {
	_n = epoll_wait(_epoll_fd, _epollClient.data(), MAX_EVENTS, -1);
	_time_out = clock();
	if (_n < 0 || g_stop == 0) {
		for (vector<struct epoll_event>::iterator i = _epollClient.begin(); i != _epollClient.end(); i++)
			close(i->data.fd);
		close (_epoll_fd);
		for (vector<int>::iterator i = _sock.begin(); i != _sock.end(); i++)
			close (*i);
		if (g_stop == 0)
			throw Error("\n\e[1;31mCrtl + C detected\e[0m\n");
		else
			throw Error("Error during epoll wait.");
	}
}

// Printing Request and Response

void Epoll::topars(string HTTPRequest, int ClientFD) {
	ofstream fd("./request", ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug request");
	debug_file(HTTPRequest, &fd, ClientFD);
	fd.close();
}

void	print_in_response(string headerHTTP, string tosend, int clientFD) {
	string response = headerHTTP + tosend;
	ofstream fd("./response", ios::app);
	if (!fd.is_open())
		throw Error("cant open outfile for debug response");
	debug_file(response, &fd, clientFD);
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
	_HTTPRequest.multipart = false;
	_HTTPRequest.print = false;
	_HTTPRequest.id.erase();
	return _HTTPRequest;
}

void Epoll::addClient(int port) {
	struct sockaddr_in	addr;
	socklen_t			addrLen = sizeof(addr);

	int client = accept(_sock[port], (struct sockaddr *)&addr, &addrLen);
	if (client == -1) {
		throw Error("500");
	}
	fcntl(client, F_SETFL, O_NONBLOCK);
	struct epoll_event new_client;
	new_client.events = EPOLLIN | EPOLLRDHUP | EPOLLHUP;
	new_client.data.fd = client;
	if (epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, client, &new_client) < 0) {
		close(client);
		throw Error("500");
	}
	_cliport[client] = _sock[port];
	_HTTPRequest[client] = newRequestClient();
}

int validToSend(string const &str) {
	if (str.find("\r\n\r\n") != str.npos)
		return 1;
	return 0;
}

int check_timeout(clock_t time) {
	if (clock() - time > 10000000)
		return 2;
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

string findSessionID(string request) {
	string null = "default";
	size_t i = request.find("session_id=");
	if (i != string::npos) {
		i += 11;
		if(request[i] != '\r' && request[i] != '\n') {
			size_t end = request.find('\r', i);
			null = request.substr(i, end - i);
			return null;
		}
	}
	return null;
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

void Epoll::reset(int fd) {
	_HTTPRequest[fd].req.clear();
	_HTTPRequest[fd].recvEnd = false;
	_HTTPRequest[fd].sendEnd = false;
	_HTTPRequest[fd].disconnect = false;
	_HTTPRequest[fd].sending = false;
	_HTTPRequest[fd].multipart = false;
	_HTTPRequest[fd].cgi = false;
	_HTTPRequest[fd].nbr_of_read = 0;
	_HTTPRequest[fd].size_to_reach = 0;
	_HTTPRequest[fd].bodysize = 0;
	_HTTPRequest[fd].uploading = false;
	_HTTPRequest[fd].print = false;
	modifEvents(fd, EPOLLIN, _epoll_fd);
}

map<int, int>::iterator Epoll::sendToClient(int clientID, Data &data, map<int, int>::iterator it) {
	string page;
	HttpRequest rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
	if (_HTTPRequest[_epollClient[clientID].data.fd].sending == false) {
		int valid = validToSend(_HTTPRequest[_epollClient[clientID].data.fd].req);
		if (valid == 1) {
			topars(_HTTPRequest[_epollClient[clientID].data.fd].req, _epollClient[clientID].data.fd);
			rq.parseAll();
			if (rq.parsingError) {
				cout << rq.parsingStrError << endl;
				_status = "400";
				reset(_epollClient[clientID].data.fd);
				return it;
			}
			if (_HTTPRequest[_epollClient[clientID].data.fd].bodysize > data.getMaxBodySize()) {
				_status = "413";
				reset(_epollClient[clientID].data.fd);
				return it;
			}
			_HTTPRequest[_epollClient[clientID].data.fd].connectionType = rq.getHeaderByKey("Connection").first.second.rawValue;
		}
		if (_HTTPRequest[_epollClient[clientID].data.fd].multipart == true)
			_HTTPRequest[_epollClient[clientID].data.fd].multipart = false;
		else if (rq.getHeaderByKey("Content-Type").first.second.rawValue == "multipart/form-data" && _HTTPRequest[_epollClient[clientID].data.fd].cgi == false && rq.getBody().empty()) {
			modifEvents(_epollClient[clientID].data.fd, EPOLLIN, _epoll_fd);
			_HTTPRequest[_epollClient[clientID].data.fd].multipart = true;
			return it;
		}
		string path = rq.getUrl();
		if (isDir(path) && path != "/") {
			sendDir(_response, path);
			_status = "200";
			reset(_epollClient[clientID].data.fd);
			return it;
		}
		for (map<string, string>::const_iterator itm = data.getRedirections().begin(); itm !=  data.getRedirections().end(); ++itm) {
			if (path == itm->first) {
				_response = "HTTP/1.1 302 Moved Temporary\r\n"
								"Location: " + itm->second + "\r\n"
								"Content-Length: 0\r\n"
								"Connection: close\r\n"
								"\r\n";
				_status = "302";
				reset(_epollClient[clientID].data.fd);
				return it;
			}
		}
		if (path == "/logout") {
			_HTTPRequest[_epollClient[clientID].data.fd].id = "default";
			path = "/";
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = false;
		}
		valid = check_timeout(_time_out);
		if (path.find("/try_login") != path.npos) {
			Client tmp = login(_HTTPRequest[_epollClient[clientID].data.fd].req, path);
			if (!tmp.getUser().empty()) {
				_ClientsData.push_back(tmp);
				_HTTPRequest[_epollClient[clientID].data.fd].id = tmp.getID();
				_HTTPRequest[_epollClient[clientID].data.fd].Loged = true;
			}
		}
		if (rq.getUrl() == "/favicon.ico") {
			reset(_epollClient[clientID].data.fd);
			return it;
		}
		for(map<string, string>::const_iterator i = data.getLocations().begin(); i != data.getLocations().end() && valid != 2 && page.empty(); i++) {
			if (path == i->first) {
				page = i->second;
				if (!checkRequestIsValid(i->first, data, rq.getMethodToString())) {
					_status = "403";
					reset(_epollClient[clientID].data.fd);
					return it;
				}
				break ;
			}
		}
		if (path.find("cgi-bin") != path.npos) {
			_HTTPRequest[_epollClient[clientID].data.fd].id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
			cgi execcgi(path, data, _response, _HTTPRequest[_epollClient[clientID].data.fd], rq, _status);
			string filename = find_filename(_HTTPRequest[_epollClient[clientID].data.fd].req);
			if (filename == "empty body") {
				reset(_epollClient[clientID].data.fd);
				return it;
			}
			if (filename.empty()) {
				_status = "415";
				reset(_epollClient[clientID].data.fd);
				return it;
			}
			data.add_upload(filename);
			if (_HTTPRequest[_epollClient[clientID].data.fd].cgi == false)
				reset(_epollClient[clientID].data.fd);
			return it;
		}
		else if (path.find("delete") != path.npos && rq.getMethodToString() == "DELETE")
			delete_file(path, data);
		else if (page.empty() && valid == 2 && _HTTPRequest[_epollClient[clientID].data.fd].uploading == false) {
			_status = "408";
			reset(_epollClient[clientID].data.fd);
			return it;
		}
		else if (page.empty()) {
			_status = "404";
			reset(_epollClient[clientID].data.fd);
			return it;
		}
		if (_HTTPRequest[_epollClient[clientID].data.fd].id.empty())
			_HTTPRequest[_epollClient[clientID].data.fd].id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
		if ((_HTTPRequest[_epollClient[clientID].data.fd].id == "default" && _HTTPRequest[_epollClient[clientID].data.fd].Loged == false) || _ClientsData.empty())
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = false;
		else
			_HTTPRequest[_epollClient[clientID].data.fd].Loged = true;
		ostringstream oss;
		if (rq.getMethodToString() == "POST" || rq.getMethodToString() == "GET") {
			if (rq.getUrl().find("downloads/") != rq.getUrl().npos && check_file_availability(rq.getUrl(), data) == false) {
				_status = "404";
				return it;
			}
			else {
				_HTTPRequest[_epollClient[clientID].data.fd].print = false;
				if (rq.getUrl() != "/download")
					_status = "200";
				else {
					_status = "200";
					_response = generate_upload_page(data._uploads);
					reset(_epollClient[clientID].data.fd);
					return it;
				}
			}
		}
		else if (rq.getMethodToString() == "DELETE") {
			_status = "200";
			reset(_epollClient[clientID].data.fd);
			return it;
		}
		int infile = open(page.c_str(), O_RDONLY);
		_HTTPRequest[_epollClient[clientID].data.fd].infile = infile;
		if (infile < 0) {
			_status = "403";
			reset(_epollClient[clientID].data.fd);
		}
		_HTTPRequest[_epollClient[clientID].data.fd].page = page;
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].id.empty())
		_HTTPRequest[_epollClient[clientID].data.fd].id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
	return (sendingFile(_epollClient[clientID].data.fd, _HTTPRequest[_epollClient[clientID].data.fd].infile, _HTTPRequest[_epollClient[clientID].data.fd].headerresponse, it));
}

map<int, int>::iterator	Epoll::sendingFile(int fd, int infile, string headerHTTP, map<int, int>::iterator it) {
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
	print_in_response(headerHTTP, tosend, fd);
	reset(fd);
	close(infile);
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

void	Epoll::print_rq(string rq) {
	string line;
	size_t end = rq.find("\r\n");
	if (end != rq.npos)
		line = rq.substr(0, end);
	cout << PURPLE + line << "  ";
}

void	Epoll::print_status(int fd) {
	if (!_HTTPRequest[fd].print) {
		debug(YELLOW, _status);
		_HTTPRequest[fd].print = true;
	}
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
					print_rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
					return it;
				}
			}
			else {
					_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
					_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
					modifEvents(_epollClient[clientID].data.fd, EPOLLIN | EPOLLOUT, _epoll_fd);
					print_rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
				return it;
			}
		}
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach >= _HTTPRequest[_epollClient[clientID].data.fd].bodysize) {
		_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
		_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
		modifEvents(_epollClient[clientID].data.fd,  EPOLLIN | EPOLLOUT, _epoll_fd);
		print_rq(_HTTPRequest[_epollClient[clientID].data.fd].req);
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
						sendingToClient(_epollClient[clientID].data.fd, data);
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
		catch (exception const &e) {
			_status = e.what();
			sendingToClient(_epollClient[clientID].data.fd, data);
		}
	}
}

void Epoll::sendingToClient(int fd, Data &data) {
	if (_HTTPRequest[fd].cgi == false && _HTTPRequest[fd].multipart == false) {
		if (_HTTPRequest[fd].Loged) {
			addLogMessage(findRightUser(_HTTPRequest[fd].id), fd);
			_response += _HTTPRequest[fd].logMsg;
		}
		Response response(_status, _response, _HTTPRequest[fd].id, data);
		print_status(fd);
		if (send(fd, _response.c_str(), _response.length(), MSG_NOSIGNAL) <= 0)
			throw Disconnect("");
		_response.clear();
	}
}