#include "Data.hpp"
#include <dirent.h>

Data::Data()
{
	// LOG(GREEN + "Constructor by Default" + NC);
}

Data::Data(const Data& other)
{
	// LOG(BLUE + "Constructor by copy" + NC);
	*this = other;
}

Data::Data(std::string const &str, char **env)
{
	cpEnv(env);
	std::ifstream inputfile(str.c_str());
	if (!inputfile.is_open())
		throw Error("Can't open the file");
	fill_info(inputfile);
	fill_uploads();
	// std::cerr << *this;
}

Data	&Data::operator=(const Data& other)
{
	LOG(PURPLE + "Assignation Operator" + NC);
	if (this != &other) {

	}
	return *this;
}

void Data::fill_info(std::ifstream &infile)
{
	std::string line;
	std::string serverNames;
	std::string ports;
	std::string loc;
	std::string err;
	while (!infile.eof())
	{
		std::getline(infile, line);
		if (line.find("port ", 0) != line.npos)
		{
			//debug(1);
			ports = line.substr(line.find("port ", 0) + 5, line.size());
			SetPorts(ports);
		}
		if (line.find("host ", 0) != line.npos)
		{
			//debug(2);
			SetHost(line.substr(line.find("host ", 0) + 5, line.size()));
		}
		else if (line.find("bodysize ", 0) != line.npos)
			_bodySize = atoi(line.c_str() + line.find("bodysize ", 0) + 9);
		else if (line.find("server_name ", 0) != line.npos)
		{
			//debug(3);
			serverNames = line.substr(line.find("server_name ", 0) + 12, line.size());
			SetServerNames(serverNames);
		}
		else if (line.find("location ", 0) != line.npos)
		{
			//debug(4);
			loc = line.substr(line.find("location ", 0) + 9, line.size());
			while (std::getline(infile, line))
			{
				loc += line;
				if (line.find("}") != line.npos)
					break;
			}
			SetLocations(loc);
		}
		else if (line.find("error_pages ", 0) != line.npos)
		{
			//debug(5);
			err = line.substr(line.find("error_pages ", 0) + 12, line.size());
			while (std::getline(infile, line))
			{
				err += line + "\n";
				if (line.find("}") != line.npos)
					break;
			}
			SetErrors(err);
		}
	}
	infile.close();
	std::ofstream file("/etc/hosts", std::ios::app);
	if (!file.is_open())
		throw Error("Can't open host file");
	for (std::vector<std::string>::iterator it = _serverNames.begin(); it != _serverNames.end(); ++it)
		file << getHostStr() << " "  <<  *it << std::endl;
	file.close();
}

std::ostream &operator<<(std::ostream &os, Data const &data)
{
	os << "this is the content of my Data class named data" << std::endl;
	os << "_port = " << data.getPort() << std::endl;
	os << "_host = " << data.getHostIP() << std::endl;
	os << "_bodySize = " << data.getBodySize() << std::endl;
	::debug_container(RED, "Servernames:", data.getServerNames(), os);
	return os;
}

unsigned long const &Data::getHostIP() const
{
	return _hostIP;
}

std::string const &Data::getHostStr() const
{
	return _hostStr;
}

int* const &Data::getPort() const
{
	return _ports;
}

int const &Data::getNbrPort() const
{
	return _nbrPorts;
}

size_t const &Data::getBodySize() const
{
	return _bodySize;
}

std::vector<std::string> const &Data::getServerNames() const
{
	return _serverNames;
}

std::map<std::string, std::string> &Data::getLocations()
{
	return _loc;
}
	
std::map<std::string, std::string> const &Data::getErrors() const
{
	return _errors;
}

std::map<std::string, std::vector<std::string> > &Data::getMethods()
{
	return _method;
}


void Data::SetHost(std::string const &hostToShift)
{
	_hostStr = hostToShift;
	std::istringstream iss(hostToShift);
	std::string segment;
	int shift = 24;

	_hostIP = 0;
	while (std::getline(iss, segment, '.'))
	{
		_hostIP |= (std::strtoul(segment.c_str(), NULL, 10) << shift);
		shift -= 8;
	}
}

void Data::SetPorts(std::string const &ports)
{
	size_t oldpos = 0;
	std::size_t pos = 0;
	int start = 0;
	int end = ports.length() - 1;
	int i = 0;
	while (isspace(ports[start]))
		start++;
	while (isspace(ports[end]))
		end--;
	//debug(6);
	std::string portsparsed = ports.substr(start, end - start + 1);
	int count = 0;
	size_t j = 0;
	while ((j = portsparsed.find(" ", j)) != std::string::npos) {
		++count;
		++j;
	}
	_ports = new int[count + 1];
	while (pos <= portsparsed.size() && pos != portsparsed.npos)
	{
		pos = portsparsed.find(' ', pos);
		if (pos != portsparsed.npos)
		{
			//debug(7);
			_ports[i] = atoi(portsparsed.substr(oldpos, pos - oldpos).c_str());
			pos += 1;
			i++;
		}
		else
		{
			//debug(8);
			_ports[i] = atoi(portsparsed.substr(oldpos, pos - oldpos).c_str());
			i++;
		}
		oldpos = pos;
	}
	_nbrPorts = i;
}

void Data::SetServerNames(std::string const &servernames)
{
	size_t oldpos = 0;
	std::size_t pos = 0;
	int start = 0;
	int end = servernames.length() - 1;
	while (isspace(servernames[start]))
		start++;
	while (isspace(servernames[end]))
		end--;
	//debug(9);
	std::string servernamesparsed = servernames.substr(start, end - start + 1);
	while (pos <= servernamesparsed.size() && pos != servernamesparsed.npos)
	{
		pos = servernamesparsed.find(' ', pos);
		if (pos != servernamesparsed.npos)
		{
			//debug(10);
			_serverNames.push_back(servernamesparsed.substr(oldpos, pos - oldpos));
			pos += 1;
		}
		else
		{
			//debug(10);
			_serverNames.push_back(servernamesparsed.substr(oldpos, servernamesparsed.length() - oldpos));
		}
		oldpos = pos;
	}
}

std::vector<std::string> Data::setMethods(const std::string &loc)
{
	std::vector<std::string> method;
	std::size_t start;
	start = loc.find("Method", 0);
	if (start  != loc.npos)
	{
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

void Data::SetLocations(std::string const &location)
{
	std::string path;
	std::string page;
	std::size_t path_start;
	std::size_t path_end;
	std::size_t page_start;
	std::size_t page_end;

	// debug(PURPLE, location);
	path_start = location.find("/");
	path_end = location.find(" ", path_start);
	//debug(11);
	if (path_end != location.npos || path_start != location.npos)
		path = location.substr(path_start, path_end - path_start);
	page_start = location.find("./");
	page_end = location.find(".html");
	//debug(12);
	if (page_end != location.npos || page_start != location.npos)
		page = location.substr(page_start, page_end - page_start + 5);

	_loc[path] = page;
	std::vector<std::string> method;
	method = setMethods(location);
	_method[path] = method;
}

void Data::SetErrors(std::string const &errors)
{
	std::string err;
	std::string page;
	std::size_t err_start;
	std::size_t page_start;
	std::size_t page_end;
	std::istringstream err_stream(errors);
	std::string line;
	while (err_stream)
	{
		std::getline(err_stream, line);
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
		//debug(13);
		err = line.substr(err_start, 3);
		//debug(14);
		page = line.substr(page_start, page_end - page_start + 5);
		_errors[err] = page;
	}
}

char **Data::envToCharpp()
{
	char **str = new char *[_env.size() + 1];
	std::string tmp;
	int i = 0;
	for (std::map<string,string>::iterator it = _env.begin(); it != _env.end(); ++it)
	{
		tmp = it->first + "=" + it->second;
		str[i++] = strdup(tmp.c_str());
	}
	str[i] = NULL;
	return str;
}

void Data::cpEnv(char **env) {
	std::string tmp;
	std::string name;
	std::string var;
	std::size_t name_end;
	std::size_t var_start;
	for (int j = 0; env[j]; j++) {
		tmp = env[j];
		name_end = tmp.find("=", 0);
		var_start = name_end + 1;
		//debug(15);
		name = tmp.substr(0, name_end);
		//debug(16);
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
		std::string filename = entry->d_name;
		if (filename == "." || filename == "..") {
            continue;
        }
		_uploads.push_back(filename);
		_loc["/downloads/" + filename] = "./site/downloads/" + filename;
		std::vector<std::string> method;
		method.push_back("GET");
		_method["/downloads/" + filename] = method;
	}
}

Data::~Data()
{
	LOG(RED + "Destructor by default" + NC);
	delete[] _ports;
}

