#include "Data.hpp"

Data::Data() {}

Data::Data(const Data& other) {
	*this = other;
}

Data::Data(string const &str, char **env) {
	cpEnv(env);
	ifstream inputfile(str.c_str());
	if (!inputfile.is_open())
		throw Error("Can't open the file");
	fill_info(inputfile);
	fill_uploads();
}

void Data::fill_info(ifstream &infile) {
	string line;
	string serverNames;
	string ports;
	string loc;
	string err;
	string redirect;
	while (!infile.eof()) {
		getline(infile, line);
		if (line.find("port ", 0) != line.npos) {
			ports = line.substr(line.find("port ", 0) + 5, line.size());
			SetPorts(ports);
		}
		if (line.find("host ", 0) != line.npos)
			SetHost(line.substr(line.find("host ", 0) + 5, line.size()));
		else if (line.find("bodysize ", 0) != line.npos)
			_MaxBodySize = atoi(line.c_str() + line.find("bodysize ", 0) + 9);
		else if (line.find("server_name ", 0) != line.npos) {
			serverNames = line.substr(line.find("server_name ", 0) + 12, line.size());
			SetServerNames(serverNames);
		}
		else if (line.find("location ", 0) != line.npos) {
			loc = line.substr(line.find("location ", 0) + 9, line.size());
			while (getline(infile, line)) {
				loc += line;
				if (line.find("}") != line.npos)
					break;
			}
			SetLocations(loc);
		}
		else if (line.find("error_pages ", 0) != line.npos) {
			err = line.substr(line.find("error_pages ", 0) + 12, line.size());
			while (getline(infile, line)) {
				err += line + "\n";
				if (line.find("}") != line.npos)
					break;
			}
			SetErrors(err);
		}
		else if (line.find("redirection") != string::npos) {
			//12 parce que "redirection " fait 12
			redirect = line.substr(12, line.find(" {") - 12);
			getline(infile, line);
			_redirect[redirect] = line.substr(1, line.length() - 1);
		}
	}
	infile.close();
	ofstream file("/etc/hosts", ios::app);
	if (!file.is_open())
		throw Error("Can't open host file");
	for (vector<string>::iterator it = _serverNames.begin(); it != _serverNames.end(); ++it)
		file << getHostStr() << " "  <<  *it << endl;
	file.close();
}

ostream &operator<<(ostream &os, Data const &data) {
	os << "this is the content of my Data class named data" << endl;
	os << "_port = " << data.getPort() << endl;
	os << "_host = " << data.getHostIP() << endl;
	os << "_bodySize = " << data.getMaxBodySize() << endl;
	::debug_container(RED, "Servernames:", data.getServerNames(), os);
	return os;
}

unsigned long const &Data::getHostIP() const {
	return _hostIP;
}

string const &Data::getHostStr() const {
	return _hostStr;
}

int* const &Data::getPort() const {
	return _ports;
}

int const &Data::getNbrPort() const {
	return _nbrPorts;
}

size_t const &Data::getMaxBodySize() const {
	return _MaxBodySize;
}

vector<string> const &Data::getServerNames() const {
	return _serverNames;
}

map<string, string> &Data::getLocations() {
	return _loc;
}
	
map<string, string> const &Data::getErrors() const {
	return _errors;
}

map<string, vector<string> > &Data::getMethods() {
	return _method;
}

map<string, string> const &Data::getRedirections() const {
	return _redirect;
}


void Data::SetHost(string const &hostToShift) {
	_hostStr = hostToShift;
	istringstream iss(hostToShift);
	string segment;
	int shift = 24;

	_hostIP = 0;
	while (getline(iss, segment, '.')) {
		_hostIP |= (strtoul(segment.c_str(), NULL, 10) << shift);
		shift -= 8;
	}
}

void Data::SetPorts(string const &ports) {
	size_t oldpos = 0;
	size_t pos = 0;
	int start = 0;
	int end = ports.length() - 1;
	int i = 0;
	while (isspace(ports[start]))
		start++;
	while (isspace(ports[end]))
		end--;
	string portsparsed = ports.substr(start, end - start + 1);
	int count = 0;
	size_t j = 0;
	while ((j = portsparsed.find(" ", j)) != string::npos) {
		++count;
		++j;
	}
	_ports = new int[count + 1];
	while (pos <= portsparsed.size() && pos != portsparsed.npos) {
		pos = portsparsed.find(' ', pos);
		if (pos != portsparsed.npos) {
			_ports[i] = atoi(portsparsed.substr(oldpos, pos - oldpos).c_str());
			pos += 1;
			i++;
		}
		else {
			_ports[i] = atoi(portsparsed.substr(oldpos, pos - oldpos).c_str());
			i++;
		}
		oldpos = pos;
	}
	_nbrPorts = i;
	int count2;
	while (i-- > 0) {
		count2 = _nbrPorts;
		while (count2-- > 0)
			if (_ports[count2] == _ports[i] && i != count2)
				throw Error("Error: 2 similar ports.");
	}
}

void Data::SetServerNames(string const &servernames) {
	size_t oldpos = 0;
	size_t pos = 0;
	int start = 0;
	int end = servernames.length() - 1;
	while (isspace(servernames[start]))
		start++;
	while (isspace(servernames[end]))
		end--;
	string servernamesparsed = servernames.substr(start, end - start + 1);
	while (pos <= servernamesparsed.size() && pos != servernamesparsed.npos) {
		pos = servernamesparsed.find(' ', pos);
		if (pos != servernamesparsed.npos) {
			_serverNames.push_back(servernamesparsed.substr(oldpos, pos - oldpos));
			pos += 1;
		}
		else
			_serverNames.push_back(servernamesparsed.substr(oldpos, servernamesparsed.length() - oldpos));
		oldpos = pos;
	}
}

vector<string> Data::setMethods(const string &loc) {
	vector<string> method;
	size_t start;
	start = loc.find("Method", 0);
	if (start  != loc.npos) {
		start = loc.find("GET", 0);
		if (start  != loc.npos)
			method.push_back("GET");
		start = loc.find("POST", 0);
		if (start  != loc.npos)
			method.push_back("POST");
		start = loc.find("DELETE", 0);
		if (start  != loc.npos)
			method.push_back("DELETE");
	}
	return method;
}

void Data::SetLocations(string const &location) {
	string path;
	string page;
	size_t path_start;
	size_t path_end;
	size_t page_start;
	size_t page_end;

	path_start = location.find("/");
	path_end = location.find(" ", path_start);
	if (path_end != location.npos || path_start != location.npos)
		path = location.substr(path_start, path_end - path_start);
	page_start = location.find("./");
	page_end = location.find(".html");
	if (page_end != location.npos || page_start != location.npos)
		page = location.substr(page_start, page_end - page_start + 5);

	_loc[path] = page;
	vector<string> method;
	method = setMethods(location);
	_method[path] = method;
}

void Data::SetErrors(string const &errors) {
	string err;
	string page;
	size_t err_start;
	size_t page_start;
	size_t page_end;
	istringstream err_stream(errors);
	string line;
	while (err_stream) {
		getline(err_stream, line);
		if (line.find("504", 0) != line.npos)
			err_start = line.find("504", 0);
		else if (line.find("4", 0) != line.npos)
			err_start = line.find("4", 0);
		else if (line.find("5", 0) != line.npos)
			err_start = line.find("5", 0);
		else
			continue;
		if (line.find("./", 0) != line.npos)
			page_start = line.find("./", 0);
		if (line.find(".html", page_start) != line.npos)
			page_end = line.find(".html", page_start);
		err = line.substr(err_start, 3);
		page = line.substr(page_start, page_end - page_start + 5);
		_errors[err] = page;
	}
}

char **Data::envToCharpp() {
	char **str = new char *[_env.size() + 1];
	string tmp;
	int i = 0;
	for (map<string,string>::iterator it = _env.begin(); it != _env.end(); ++it) {
		tmp = it->first + "=" + it->second;
		str[i++] = strdup(tmp.c_str());
	}
	str[i] = NULL;
	return str;
}

void Data::cpEnv(char **env) {
	string tmp;
	string name;
	string var;
	size_t name_end;
	size_t var_start;
	for (int j = 0; env[j]; j++) {
		tmp = env[j];
		name_end = tmp.find("=", 0);
		var_start = name_end + 1;
		name = tmp.substr(0, name_end);
		var = tmp.substr(var_start, tmp.size());
		_env[name] = var;
	}
}

void Data::fill_uploads() {
	DIR* dir = opendir("./site/downloads/");
	if (!dir)
		throw Error("Can't open downloads directory.");
	
	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL) {
		string filename = entry->d_name;
		if (filename == "." || filename == "..") {
            continue;
        }
		_uploads.push_back(filename);
		_loc["/downloads/" + filename] = "./site/downloads/" + filename;
		vector<string> method;
		method.push_back("GET");
		_method["/downloads/" + filename] = method;
	}
	closedir(dir);
}

void Data::add_upload(string filename) {
	vector<string>::iterator it;
	for (it = _uploads.begin(); it != _uploads.end(); it++) {
		if (filename == *it)
			break ;
	}
	if (it == _uploads.end()) {
		_uploads.push_back(filename);
	}
	_loc["/downloads/" + filename] = "./site/downloads/" + filename;
	vector<string> method;
	method.push_back("GET");
	_method["/downloads/" + filename] = method;
}

Data::~Data() {
	LOG(RED + "Destructor by default" + NC);
	delete[] _ports;
}

