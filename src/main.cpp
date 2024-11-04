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

int main()
{
	try {
		signal(SIGINT, signal_handler);
		debug("Starting...");
		debug("Server Socket...");
		int port[3] = {4343, 4444, 4545};
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, port, INADDR_ANY, 10);
		Epoll epoll(servSock.getSock());
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