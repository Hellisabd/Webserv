#include "Data.hpp"

Data::Data()
{
	// LOG(GREEN + "Constructor by Default" + NC);
}

Data::Data(const Data& other)
{
	// LOG(BLUE + "Constructor by copy" + NC);
	*this = other;
}

Data::Data(std::string const &str)
{
	std::ifstream inputfile(str.c_str());
	if (!inputfile.is_open())
		throw Error("Can't open the file");
	fill_info(inputfile);
	std::cerr << *this;
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
	while (!infile.eof())
	{
		std::getline(infile, line);
		if (line.find("port ", 0) != line.npos)
		{
			ports = line.substr(line.find("port ", 0) + 5, line.size());
			SetPorts(ports);
		}
		else if (line.find("host ", 0) != line.npos)
			setHost(line.substr(line.find("host ", 0) + 5, line.size()));
		else if (line.find("bodysize ", 0) != line.npos)
			_bodySize = atoi(line.c_str() + line.find("bodysize ", 0) + 9);
		else if (line.find("server_name ", 0) != line.npos)
		{
			serverNames = line.substr(line.find("server_name ", 0) + 12, line.size());
			SetServerNames(serverNames);
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

void Data::setHost(std::string const &hostToShift)
{
	_hostStr = hostToShift;
	debug(GREEN, hostToShift);
	std::istringstream iss(hostToShift);
	std::string segment;
	int shift = 24;

	_hostIP = 0;
	while (std::getline(iss, segment, '.'))
	{
		debug(GREEN, segment);
		_hostIP |= (std::stoul(segment) << shift);
		debug(BLUE, _hostIP);
		shift -=8;
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
	std::string portsparsed = ports.substr(start, end - start + 1);
	while (pos <= portsparsed.size() && pos != portsparsed.npos)
	{
		pos = portsparsed.find(' ', pos);
		if (pos != portsparsed.npos)
		{
			_ports[i] = atoi(portsparsed.substr(oldpos, pos - oldpos).c_str());
			pos += 1;
			i++;
		}
		else
		{
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
	std::string servernamesparsed = servernames.substr(start, end - start + 1);
	while (pos <= servernamesparsed.size() && pos != servernamesparsed.npos)
	{
		// ::debug(RED, "pos: ", "prout");
		pos = servernamesparsed.find(' ', pos);
		if (pos != servernamesparsed.npos)
		{
			_serverNames.push_back(servernamesparsed.substr(oldpos, pos - oldpos));
			pos += 1;
		}
		else
			_serverNames.push_back(servernamesparsed.substr(oldpos, servernamesparsed.length() - oldpos));
		oldpos = pos;
	}
}

Data::~Data()
{
	LOG(RED + "Destructor by default" + NC);
}