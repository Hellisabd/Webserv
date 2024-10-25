#include "webserv.hpp"


void test(Data &data)
{
	int server_fd = socket(AF_UNSPEC, SOCK_STREAM, 0);
	if (server_fd < 0)
		throw Error("error while  creating socket");
	struct sockaddr_in address;


	

}

int main(int argc, char **argv)
{
	try
	{
		if (argc != 2)
			Error("Wrong number of arguments");
		Data data(argv[1]);
		test(data);
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl; 
	};
}