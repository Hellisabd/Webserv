#include "Data.hpp"

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
		if (line.find("port:", 0) != line.npos)
			_port = atoi(line.c_str() + line.find("port:", 0) + 5);
		else if (line.find("host ", 0) != line.npos)
			_host = line.substr(line.find("host ", 0) + 5, line.size());
		else if (line.find("bodysize ", 0) != line.npos)
			_bodySize = atoi(line.c_str() + line.find("bodysize:", 0) + 9);
		else if (line.find("server_name ", 0) != line.npos)
		{
			serverNames = _host = line.substr(line.find("server_name ", 0) + 12, line.size());
			// SetServerNames(serverNames);
		}
	}
}

void Data::printer()
{
	std::cout << "this is the content of my Data class named data" << std::endl;
	std::cout << "_port = " << _port << std::endl;
	std::cout << "_host = " << _host << std::endl;
	std::cout << "_bodySize = " << _bodySize << std::endl;
	for (std::vector<std::string>::iterator it = _serverNames.begin(); it != _serverNames.end();++it)
		std::cout << "_server_names = " << *it << std::endl;

}

void Data::SetServerNames(std::string const &servernames)
{
	for (int pos = 0; pos != servernames.size(); )
	{
		if (pos = servernames.find(pos, ))
	}
	
}

Data::~Data()
{
	LOG(RED + "Destructor by default" + NC);
}