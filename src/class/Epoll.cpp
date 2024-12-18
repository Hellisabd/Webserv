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
	_ClientSock.resize(MAX_EVENTS);
	for (vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++)
		*i = -1;
}

Epoll::~Epoll() {
	for (vector<int>::iterator i = _ClientSock.begin(); i != _ClientSock.end(); i++) {
		if (*i != -1)
			close(*i);
	}
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
	return _HTTPRequest;
}

void Epoll::addClient(int port) {
	int client = accept(_sock[port], NULL, NULL);
	if (client == -1) {
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
}

int validToSend(string const &str) {
	if (str.find("\r\n\r\n") != str.npos)
		return 1;
	return 0;
}

int check_timeout(clock_t time, string url) {
	if (clock() - time > 10000 && url != "/upload")
		return 2;
	return 0;
}

string getScriptName(string url) {
	size_t start;
	size_t end;
	start = url.find("cgi-bin/", 0);
	if (start == url.npos)
		return "";
	end = url.find("/", start + 8);
	if (end == url.npos)
		return "./" + url.substr(start, url.length() - start);
	else
		return "./" + url.substr(start, end - start);
}

void Epoll::set_new_env(Data &data, HttpRequest rq) {
	data._env["PATH_INFO"] = rq.getUrl();
	data._env["SCRIPT_NAME"] = getScriptName(rq.getUrl());
	data._env["REQUEST_METHOD"] = rq.getMethod();
}

map<int, int>::iterator Epoll::exec(Data &data, int clientID, HttpRequest rq, string req_str, map<int, int>::iterator it, string id) {
	int fd[2];
	if (pipe(fd) == -1)
		return it;
	string text;
	if (req_str.find("text=") != req_str.npos) {
		size_t start = req_str.find("text=") + 5;
		if (start != req_str.npos)
			text = req_str.substr(start, req_str.length() - start);
		if (text.length() > 18000)
			text = "Text too long.";
		replace(text);
		data._env["text"] = text;
	}
	int pid = fork();
	if (pid == -1) {
		close (fd[0]);
		close (fd[1]);
		return it;
	}
	set_new_env(data, rq);
	if (pid == 0) {
		char **env;
		env = data.envToCharpp();
		if (-1 == dup2(fd[1], STDOUT_FILENO)) {
			close (fd[0]);
			close (fd[1]);
			return it;
		}
		close(fd[0]);
		close(fd[1]);
		char **filename = new char*[2];
		if (rq.getUrl().find("script.php") != rq.getUrl().npos) {
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
	if (byte_read < 0) {
		close (fd[0]);
		close (fd[1]);
		return it;
	}
	close(fd[0]);
	close(fd[1]);
	buf[byte_read] = '\0';
	ostringstream oss;
	oss << byte_read;
	string headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
	if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
		throw Error("Error sending HTTP header");
	if (send(_epollClient[clientID].data.fd, buf, byte_read, MSG_NOSIGNAL) < 0) {
		perror("exec send body");
		throw Error("");
	}
	_HTTPRequest[_epollClient[clientID].data.fd].req.clear();
	_HTTPRequest[_epollClient[clientID].data.fd].recvEnd = false;
	_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
	_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
	_HTTPRequest[_epollClient[clientID].data.fd].bodysize = 0;
	modifEvents(_epollClient[clientID].data.fd, EPOLLIN, _epoll_fd);
	if (_HTTPRequest[_epollClient[clientID].data.fd].connectionType != "keep-alive")
		it = deleteClient(it);
	return it;
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
	string null = "\r\n";
	size_t i = request.find("session_id=");
	if (i != string::npos)
	{
		i += 11;
		if(request[i] != '\r' || request[i] != '\n')
		{
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

void addLogMessage(string page, string user) {
	std::ifstream inputFile(page.c_str());
	if (!inputFile.is_open()) {
		std::cerr << "Error: Unable to open file " << page << std::endl;
		return;
	}
	std::string content((std::istreambuf_iterator<char>(inputFile)), std::istreambuf_iterator<char>());
	inputFile.close();

	std::string loginBanner = 
	"<div style=\"position: absolute; top: 10px; right: 10px; "
	"background-color: #f0f0f0; padding: 5px 10px; border: 1px solid #ccc; "
	"border-radius: 5px; font-family: Arial, sans-serif;\">\n"
	"    Log as: " + user + "\n"
	"</div>\n";

	content += "\n" + loginBanner;
	std::ofstream outputFile(page.c_str(), std::ios::trunc);
	if (!outputFile.is_open()) {
		std::cerr << "Error: Unable to open file for writing " << page << std::endl;
	return;
	}
	outputFile << content;
	outputFile.close();
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
			debug("rq bodysize: ", rq.getContentLength());
			debug("data bodysize: ", rq.getBodySize());
			if (_HTTPRequest[_epollClient[clientID].data.fd].bodysize > data.getBodySize())
			{
				page = data.getErrors().find("413")->second;
			}
			_HTTPRequest[_epollClient[clientID].data.fd].connectionType = rq.getHeaderByKey("Connection").first.second.rawValue;
		}
		string path = rq.getUrl();
		valid = check_timeout(_time_out, path);
		if (path.find("/try_login") != path.npos) {
			Client tmp = login(_HTTPRequest[_epollClient[clientID].data.fd].req, path);
			if (!tmp.getUser().empty())
			{
				_ClientsData.push_back(tmp);
				id = tmp.getID();
			}
		}
		if (path.find("/upload") != path.npos && rq.getMethodToString() == "POST") {
			string filename = uploadFile(_HTTPRequest[_epollClient[clientID].data.fd].req, data, &_HTTPRequest[_epollClient[clientID].data.fd].uploading);
			if (filename == "415")
				page = data.getErrors().find(filename)->second;
			string tmp_name;
			if (filename.find("/downloads") != filename.npos)
				tmp_name = filename.substr(filename.find("/downloads") + 11, filename.length() - (filename.find("/downloads")) + 11);
			vector<string> method;
			method.push_back("GET");
			map<string, vector<string> > &tmp = data.getMethods();
			tmp["/downloads/" + tmp_name] = method;
			map<string, string> &tmploc = data.getLocations();
			if (filename.find("/downloads") != filename.npos)
				tmploc["/downloads/" + tmp_name] = filename;
			generate_uploads_url(data._uploads);
		}
		if (rq.getUrl() == "/favicon.ico"){
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
			if (page != "./site/cgi.html")
				page = data.getErrors().find("404")->second;
			else {
				id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
				it = exec(data, clientID, rq, _HTTPRequest[_epollClient[clientID].data.fd].req, it, id);
			}
		}
		else if (path.find("delete") != path.npos && rq.getMethodToString() == "DELETE") {
			delete_file(path, data);
			generate_uploads_url(data._uploads);
		}
		else if (page.empty() && valid == 2 && _HTTPRequest[_epollClient[clientID].data.fd].uploading == false)
		{
			page = data.getErrors().find("408")->second;
		}
		else if (page.empty())
			page = data.getErrors().find("404")->second;
		if (_ClientsData.empty())
			id = "default";
		else if (id.empty()) {
			id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
		}
		ostringstream oss;
		if (rq.getMethodToString() == "POST" || rq.getMethodToString() == "GET") {
			string headerHTTP;
			if (rq.getUrl().find("downloads/") != rq.getUrl().npos && check_file_availability(rq.getUrl(), data) == false) {
				page = data.getErrors().find("404")->second;
				oss << getFileSize(page);
				// if (log)
					// _HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send = getFileSize(page) + taille balise log;
				_HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send = getFileSize(page);
				headerHTTP = "HTTP/1.1 404 Not Found\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
			}
			else {
				if (id != "default")
					addLogMessage(page, findRightUser(id));
				oss << getFileSize(page);
				_HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send = getFileSize(page);
				headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
			}
			_HTTPRequest[_epollClient[clientID].data.fd].headerresponse = headerHTTP;
			if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
				throw Error("Error sending HTTP header");
		}
		else if (rq.getMethodToString() == "DELETE") {
			string headerHTTP = "HTTP/1.1 200 OK\r\nSet-Cookie: session_id=" + id + "; Path=/; HttpOnly\r\nContent-Type: text/html\r\nContent-Length: " + oss.str() + "\r\n\r\n";
			_HTTPRequest[_epollClient[clientID].data.fd].headerresponse = headerHTTP;
			if (send(_epollClient[clientID].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0) < 0)
				throw Error("Error sending HTTP header");
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
		int infile = open(page.c_str(), O_RDONLY);
		_HTTPRequest[_epollClient[clientID].data.fd].infile = infile;
		if (infile < 0)
			page = data.getErrors().find("403")->second;
		if (page == "./site/submit_comment.html") {
			save_comment(_HTTPRequest[_epollClient[clientID].data.fd].req);
			generate_comment_page();
		}
		_HTTPRequest[_epollClient[clientID].data.fd].page = page;
	}
	if (id.empty())
		id = findSessionID(_HTTPRequest[_epollClient[clientID].data.fd].req);
	return (sendingFile(_epollClient[clientID].data.fd, _HTTPRequest[_epollClient[clientID].data.fd].infile, _HTTPRequest[_epollClient[clientID].data.fd].headerresponse, _HTTPRequest[_epollClient[clientID].data.fd].size_of_file_to_send, it, id));
}

map<int, int>::iterator	Epoll::sendingFile(int fd, int infile, string headerHTTP, size_t size_to_send, map<int, int>::iterator it, string id) {
	char tosend[1024];
	ssize_t file_read;
	_HTTPRequest[fd].sending = true;
	file_read = read(infile, tosend, sizeof(tosend));
	_HTTPRequest[fd].size_to_reach += file_read;
	if (file_read < 1024)
		tosend[file_read] = '\0';
	if (send(fd, tosend, file_read, MSG_NOSIGNAL) < 0) {
		close(infile);
		perror("client send body");
		throw Error("");
	}
	if (_HTTPRequest[fd].size_to_reach >= size_to_send) {
		if (id != "default")
			clearLogMsg(_HTTPRequest[fd].page);
		print_in_response(headerHTTP, tosend, fd);
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
		if (_HTTPRequest[fd].connectionType != "keep-alive")
			it = deleteClient(it);
	}
	return it;
}

size_t getbodysize(string str, size_t start) {
	return atoi(str.c_str() + start);
}

void Epoll::modifEvents(int fd, int event, int epoll_fd) {
	struct epoll_event ev;
	ev.events = event | EPOLLRDHUP | EPOLLHUP;
	ev.data.fd = fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
}

void Epoll::readFromClient(int clientID) {
	char buffer[1025];
	ssize_t bytes_read = 0;
	bytes_read = read(_epollClient[clientID].data.fd, buffer, 1024);
	if (bytes_read < 0)
		return ;
	buffer[bytes_read] = '\0';
	if (bytes_read > 0) {
		_HTTPRequest[_epollClient[clientID].data.fd].req.append(buffer, bytes_read);
		if (_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read != 0)
			_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read++;
		_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach += bytes_read;
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
					modifEvents(_epollClient[clientID].data.fd, EPOLLOUT, _epoll_fd);
					return ;
				}
			}
			else {
					_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
					_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
				modifEvents(_epollClient[clientID].data.fd, EPOLLOUT, _epoll_fd);
				return ;
			}
		}
	}
	if (_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach >= _HTTPRequest[_epollClient[clientID].data.fd].bodysize) {
		_HTTPRequest[_epollClient[clientID].data.fd].nbr_of_read = 0;
		_HTTPRequest[_epollClient[clientID].data.fd].size_to_reach = 0;
		// debug(_HTTPRequest[_epollClient[clientID].data.fd].req);
		modifEvents(_epollClient[clientID].data.fd, EPOLLOUT, _epoll_fd);
	}
}

map<int, int>::iterator Epoll::deleteClient(map<int, int>::iterator it) {
	epoll_ctl(_epoll_fd, EPOLL_CTL_DEL, it->first, &_epollServ);
	for (vector<int>::iterator fd = _ClientSock.begin(); fd != _ClientSock.end(); ++fd) {
		if (*fd == it->first) {
			_ClientSock.erase(fd);
			break;
		}
	}
	_noclient = true;
	close(it->first);
	map<int, int>::iterator next_it = it;
	++next_it;
	_cliport.erase(it);
	return next_it;
}

bool Epoll::isSockPort(int fd) {
	for (vector<int>::iterator it = _sock.begin(); it != _sock.end(); ++it) {
		if (fd == *it)
			return true;
	}
	return false;
}

void Epoll::handleRequest(Data &data) {
	map<int, int>::iterator it = _cliport.begin();
	_noclient = false;
	for (int clientID = 0; clientID < _n; clientID++) {
		for (size_t port = 0; port < _sock.size(); port++) {
			if (it == _cliport.end() && _noclient) {
				_noclient = false;
				break;
			}
			if (_epollClient[clientID].data.fd == _sock[port]) {
				addClient(port);
			}
			else if (it->second == _sock[port] && !isSockPort(_epollClient[clientID].data.fd)) {
				if (_epollClient[clientID].events & (EPOLLHUP | EPOLLRDHUP)) {
					it = deleteClient(it);
					break;
				}
				else if (_epollClient[clientID].events & EPOLLIN) {
					readFromClient(clientID);
					// debug (clientID);
					break;
				}
				else if (_epollClient[clientID].events & EPOLLOUT) {
					it = sendToClient(clientID, data, it);
					break ;
				}
			}
			if (_noclient == true) {
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