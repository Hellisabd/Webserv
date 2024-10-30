#include "webserv.hpp"

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

//Bonne chance pour capter qqch mec! 

int main()
{
	try
	{
		signal(SIGINT, signal_handler);
		debug("Starting...");
		debug("Server Socket...");
		int port[2] = {4343, 4444};
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, port, INADDR_ANY, 10);
		Epoll epoll(servSock.getSock());
		try {
			while (g_stop) {
				epoll.wait(g_stop);
				epoll.add(servSock.getAddr());
			}
		}
		catch (std::exception const &e)
		{
		std::cerr << e.what() << std::endl;
		}
	}
		
		// }
		// catch(const std::exception& e)
		// {
		// 	for (int i = 0; i < nbr_of_client; i++)
		// 	{
		// 		if (client_fd[i] != servSock.getSock())
		// 			close (client_fd[i]);
		// 	}
		// 	std::cerr << e.what() << '\n';
		// }
		// for (int i = 0; i < nbr_of_client; i++)
		// {
		// 	if (client_fd[i] != servSock.getSock())
		// 		close (client_fd[i]);
		// }
		// if (infile != -1)
		// 	close(infile);
		// debug(GREEN, "SUCCESS");
		// close(epoll_fd);
		
	// }
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl;
	}
}