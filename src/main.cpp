#include "webserv.hpp"
#define PORT 4343
int g_stop = 1;

void signal_handler(int sig)
{
	if (sig == SIGINT) {
		g_stop = 0;
	}
}

unsigned long getFileSize(std::string const &file_path)
{
	struct stat file_stat;
	if (stat(file_path.c_str(), &file_stat) != 0)
		throw Error("failed to get file size");
	return file_stat.st_size;
}

int main(int argc, char **argv)
{
		try {
      if (argc != 2)
			  throw Error("Wrong number of arguments");
			signal(SIGINT, signal_handler);
			debug("Starting...");
			debug("Server Socket...");
			Data data(argv[1]);
			ServerSocket servSock(AF_INET, SOCK_STREAM, 0, data.getPort(),  data.getHostIP(), 10, data.getNbrPort());
			Epoll epoll(servSock.getSock(), data.getNbrPort());
			try {
				while (g_stop) {
					epoll.wait(g_stop);
					epoll.handleRequest(servSock.getAddr());
				}
			}
			catch (std::exception const &e) {
				std::cerr << e.what() << std::endl;
			}

		}
		catch (std::exception const &e) {
			std::cerr << e.what() << std::endl;
		}
	}
}