#include "webserv.hpp"

#define PORT 4343
#define MAX_EVENTS 10

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
		int infile = -1;
		signal(SIGINT, signal_handler);
		int nbr_of_client = 0;
		int client_fd[MAX_EVENTS];
		for (int i = 0; i < MAX_EVENTS; i++)
			client_fd[i] = -1;
		debug("Starting...");
		debug("Server Socket...");
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, 4343, INADDR_ANY, 10);
		int opt = 1;
		if (setsockopt(servSock.getSock(), SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
			throw Error("setsockopt(SO_REUSEADDR) failed");
		int epoll_fd = epoll_create(MAX_EVENTS);
		if (epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
		
		struct epoll_event epollServ;
		epollServ.events = EPOLLIN;
		epollServ.data.fd = servSock.getSock();
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, servSock.getSock(), &epollServ) == -1)
			throw Error("Error during epoll ctl");
		
		struct epoll_event epollClient[MAX_EVENTS];
		try
		{
			while (g_stop)
			{
				int n = epoll_wait(epoll_fd, epollClient, MAX_EVENTS, -1);
				if (n < 0 || !g_stop) {
					close (servSock.getSock());
					close (epoll_fd);
					throw Error("Error during epoll_wait");
				}
				for (int i = 0; i < n; i++)
				{
					if (epollClient[i].data.fd == servSock.getSock())
					{
						client_fd[nbr_of_client] = accept(servSock.getSock(), NULL, NULL);
						if (client_fd[nbr_of_client] == -1)
							throw Error("Failed to accept client connexion");
						struct epoll_event new_client;
						new_client.events = EPOLLIN;
						new_client.data.fd = client_fd[nbr_of_client];
						if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd[nbr_of_client], &new_client) < 0)
						{
							close (client_fd[nbr_of_client]);
							throw Error("Error when adding new client to epoll");
						}
						debug(GREEN, "New client added");
						nbr_of_client++;
					}
					else
					{
						char buffer[20000];
						if (read(epollClient[i].data.fd, buffer, sizeof(buffer)) <= 0)
						{
							close(epollClient[i].data.fd);
							debug(BLUE, "Client has disconnected");
						}
						else
						{
							debug(buffer);
							std::string headerHTTP = "HTTP/1.1 200 OK\r\n";
										headerHTTP += "Content-Type: text/html\r\n";
										headerHTTP += "Connection: close\r\n";
										headerHTTP += "Content-Length" + std::to_string(getFileSize("./site/index.html")) + "\r\n\r\n";
							if (0 > send(epollClient[i].data.fd, headerHTTP.c_str(), headerHTTP.size(), 0))
								throw Error("Error while sending header http");

							//buffer de 1024 pour ne pas depasser la taille du tampon et garder des perfs sur plusieurs clients simultanés
							char tosend[1024];
							infile = open("./site/index.html", O_RDONLY);
							
							size_t bytes_read = 1;
							while ((bytes_read = read(infile, tosend, sizeof(tosend))))
							{
								if (bytes_read > 0)
								{
									if (bytes_read < 1024)
										tosend[bytes_read] = '\0';
									// debug(tosend);
									send(epollClient[i].data.fd, tosend, bytes_read, 0);
								}
							}
							close(infile);
							infile = -1;
						}
					}
				}
			}
		}
		catch(const std::exception& e)
		{
			for (int i = 0; i < nbr_of_client; i++)
			{
				if (client_fd[i] != servSock.getSock())
					close (client_fd[i]);
			}
			std::cerr << e.what() << '\n';
		}
		for (int i = 0; i < nbr_of_client; i++)
		{
			if (client_fd[i] != servSock.getSock())
				close (client_fd[i]);
		}
		if (infile != -1)
			close(infile);
		debug(GREEN, "SUCCESS");
		close(epoll_fd);
		
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl;
	}
}