template<typename T>
void	debug(T content)
{
	std::cerr << RED << content << NC << std::endl;
}

template<typename T>
void	debug(std::string color, T content)
{
	std::cerr << color << content << NC << std::endl;
}

template<typename T>
void	debug_file(T content, std::ofstream *fd)
{
	*fd << "Start of the request" << std::endl;
	*fd  << content << std::endl;
	*fd << "End of the request";
	*fd << std::endl << std::endl<< std::endl<< std::endl<< std::endl<< std::endl<< std::endl<< std::endl;
}

template<typename T>
void	debug(std::string color, std::string msg, T content)
{
	if (msg.empty())
		std::cerr << color << content << NC << std::endl;
	else
		std::cerr << color + msg << content << NC << std::endl;
}

template<typename T>
void	debug_container(std::string color, std::string msg, T content, std::ostream &os)
{
	std::string spaces;
	for (std::size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		os << color + msg << std::endl;
	else
		os << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		os << spaces + *it << std::endl;
	os << NC;
}

template<typename T>
void	debug_container(std::string color, std::string msg, T content)
{
	std::string spaces;
	for (std::size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		std::cerr << color + msg << std::endl;
	else
		std::cerr << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		std::cerr << spaces << *it << std::endl;
	std::cerr << NC;
}