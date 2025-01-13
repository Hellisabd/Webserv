#include "webserv.hpp"

int g_stop = 1;

void signal_handler(int sig) {
	if (sig == SIGINT) {
		g_stop = 0;
	}
}

int main(int argc, char **argv, char **env) {
	try {
		if (argc != 2)
			throw Error("Wrong number of arguments");
		signal(SIGINT, signal_handler);
		Data data(argv[1], env);
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, data.getPort(),  data.getHostIP(), 10, data.getNbrPort());
		Epoll epoll(servSock.getSock(), data.getNbrPort());
		while (g_stop) {
			try {
				epoll.wait();
				epoll.handleRequest(data);
			}
			catch (exception const &e) {
				cerr << e.what() << endl;
			}
		}
	}
	catch (exception const &e) {
		cerr << e.what() << endl;
	}
}
