template<typename T>
void	debug(T content)
{
	cerr << RED << content << NC << endl;
}

template<typename T>
void	debug(string color, T content)
{
	cerr << color << content << NC << endl;
}

template<typename T>
void	debug_file(T content, ofstream *fd, int fd_to_print)
{
	*fd << "Start of the request for fd: " << fd_to_print << endl;
	*fd  << content << endl;
	*fd << "End of the request";
	*fd << endl << endl<< endl<< endl<< endl<< endl<< endl<< endl;
}

template<typename T>
void	debug(string color, string msg, T content)
{
	if (msg.empty())
		cerr << color << content << NC << endl;
	else
		cerr << color + msg << content << NC << endl;
}

template<typename T>
void	debug_container(string color, string msg, T content, ostream &os)
{
	string spaces;
	for (size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		os << color + msg << endl;
	else
		os << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		os << spaces + *it << endl;
	os << NC;
}

template<typename T>
void	debug_container(string color, string msg, T content)
{
	string spaces;
	for (size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		cerr << color + msg << endl;
	else
		cerr << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		cerr << spaces << *it << endl;
	cerr << NC;
}

template<typename T>
void	debug_map(string color, string msg, T content)
{
	string spaces;
	for (size_t i = 0; i < msg.length() + 1; i++)
		spaces += ' ';
	spaces += "- ";
	if (!msg.empty())
		cerr << color + msg << endl;
	else
		cerr << color;
	for (typename T::iterator it = content.begin(); it != content.end(); ++it)
		cerr << spaces << it->first << " : " << it->second << endl;
	cerr << NC;
}