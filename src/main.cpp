#include "webserv.hpp"

#define PORT 4343
#define MAX_EVENTS 10


int main()
{
	try
	{
		int nbr_of_client = 0;
		int client_fd[MAX_EVENTS];
		for (int i = 0; i < MAX_EVENTS; i++)
			client_fd[i] = -1;
		debug("Starting...");
		debug("Server Socket...");
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, 4343, INADDR_ANY, 10);
		int epoll_fd = epoll_create(MAX_EVENTS);
		if (epoll_fd < 0)
			throw Error("Error during creation of epoll_fd");
		
		struct epoll_event epollServ;
		epollServ.events = EPOLLIN;
		epollServ.data.fd = servSock.getSock();
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, servSock.getSock(), &epollServ) == -1)
			throw Error("Error during epoll ctl");
		
		struct epoll_event epollClient[MAX_EVENTS];
		while (1)
		{
			int n = epoll_wait(epoll_fd, epollClient, MAX_EVENTS, -1);
			if (n < 0)
				throw Error("Error during epoll_wait");
			for (int i = 0; i < n; i++)
			{
				if (epollClient[i].data.fd == servSock.getSock())
				{
					client_fd[nbr_of_client] = accept(servSock.getSock(), NULL, NULL);
					if (client_fd[nbr_of_client] == -1)
						throw Error("Faile to accept client connexion");
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
						debug(PURPLE, buffer);
					}
				}
			}
		}
		for (int i = 0; i < nbr_of_client; i++)
		{
			if (client_fd[i] != servSock.getSock())
				close (client_fd[i]);
		}
		debug(GREEN, "SUCCESS");
		close(epoll_fd);
		
	}
	catch (std::exception const &e)
	{
		std::cerr << e.what() << std::endl;
	}
}