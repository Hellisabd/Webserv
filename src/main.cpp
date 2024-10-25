#include "webserv.hpp"

int main(int argc, char **argv)
{
	try
	{
		if (argc != 2)
			throw Error("Wrong number of arguments");
		Data data(argv[1]);
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl; 
	};
}