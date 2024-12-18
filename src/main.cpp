#include "webserv.hpp"
#define PORT 4343
int g_stop = 1;

void signal_handler(int sig)
{
	if (sig == SIGINT) {
		g_stop = 0;
	}
}

unsigned long getFileSize(string const &file_path)
{
	struct stat file_stat;
	// debug(YELLOW, "file_path: ", file_path);
	if (stat(file_path.c_str(), &file_stat) != 0)
		throw Error("failed to get file size");
	return file_stat.st_size;
}

int main(int argc, char **argv, char **env)
{
	try {
		if (argc != 2)
				throw Error("Wrong number of arguments");
		signal(SIGINT, signal_handler);
		Data data(argv[1], env);
		ServerSocket servSock(AF_INET, SOCK_STREAM, 0, data.getPort(),  data.getHostIP(), 10, data.getNbrPort());
		Epoll epoll(servSock.getSock(), data.getNbrPort());
		generate_comment_page();
		generate_uploads_url(data._uploads);
		try {
			while (g_stop) {
				epoll.wait(g_stop);
				epoll.handleRequest(/* servSock.getAddr(),  */data);
			}
		}
		catch (exception const &e) {
			//500 internal server error
			cerr << e.what() << endl;
		}
	}
	catch (exception const &e) {
		cerr << e.what() << endl;
	}
}
