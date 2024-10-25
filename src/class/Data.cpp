#include <Data.hpp>

Data::Data()
{
	LOG(GREEN + "Constructor by Default" + NC);
}

Data::Data(const Data& other)
{
	LOG(BLUE + "Constructor by copy" + NC);
	*this = other;
}

Data::Data(std::string const &str)
{
	std::ifstream inputfile(str.c_str());
	if (!inputfile.is_open())
		throw Error("Can't open the file");
	fill_info(inputfile);
	printer();
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
	while (!infile.eof())
	{
		std::getline(infile, line);
		if (line.find("port ", 0) != line.npos)
			_port = atoi(line.c_str() + line.find("port ", 0) + 5);
		else if (line.find("host ", 0) != line.npos)
			_host = line.substr(line.find("host ", 0) + 5, line.size());
		else if (line.find("bodysize ", 0) != line.npos)
			_bodySize = atoi(line.c_str() + line.find("bodysize ", 0) + 9);
		else if (line.find("server_name ", 0) != line.npos)
		{
			serverNames = line.substr(line.find("server_name ", 0) + 12, line.size());
			SetServerNames(serverNames);
		}
	}
	infile.close();
}

void Data::printer()
{
	std::cout << "this is the content of my Data class named data" << std::endl;
	std::cout << "_port = " << _port << std::endl;
	std::cout << "_host = " << _host << std::endl;
	std::cout << "_bodySize = " << _bodySize << std::endl;
	::debug_container(RED, "Servernames:", _serverNames);
}

std::string const &Data::getHost() const
{
	return _host;
}

int const &Data::getPort() const
{
	return _port;
}

size_t const &Data::getBodySize() const
{
	return _bodySize;
}

std::vector<std::string> const &Data::getServerNames() const
{
	return _serverNames;
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
		::debug(RED, "pos: ", "prout");
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