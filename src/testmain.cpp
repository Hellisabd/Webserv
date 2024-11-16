#include <iostream>
#include "HttpRequest.hpp"

int main(void) {
	// std::string dummyRq =
	// 	"POST / HTTP/1.1\r\n"
	// 	"Host: 127.0.0.1:4444\r\n"
	// 	"User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:131.0) Gecko/20100101 Firefox/131.0\r\n"
	// 	"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/png,image/svg+xml,*/*;q=0.8\r\n"
	// 	"Accept-Language: en-US,en;q=0.5\r\n"
	// 	"Accept-Encoding: gzip, deflate, br, zstd\r\n"
	// 	"Connection: keep-alive\r\n"
	// 	"Upgrade-Insecure-Requests: 1\r\n"
	// 	"Sec-Fetch-Dest: document\r\n"
	// 	"Sec-Fetch-Mode: navigate\r\n"
	// 	"Sec-Fetch-Site: none\r\n"
	// 	"Sec-Fetch-User: ?1\r\n"
	// 	"Priority: u=0, i\r\n"
	// 	"\r\n";
	#define endlou endl
	std::string dummyRq =
		"POST / HTTP/1.1\r\nHa: lol \r\n"
		"HoSt: 127.0.0.1:4444\r\n\r\nhaha\r\n\r\n";

	// std::string dummyRq =
 //    "POST / HTTP/1.1\r\n"
 //    "Host: lol:4444\r\n"
 //    "User-Agent: curl/7.81.0\r\n"
 //    "Accept: */*\r\n"
 //    "Content-Length: 2168\r\n"
 //    "Content-Type: multipart/form-data; boundary=------------------------db7a91874317ec7e\r\n"
 //    "\r\n"
 //    "--------------------------db7a91874317ec7e\r\n"
 //    "Content-Disposition: form-data; name=\"file1\"; filename=\"main.c\"\r\n"
 //    "Content-Type: application/octet-stream\r\n"
 //    "\r\n"
 //    "#include <stdio.h>\n"
 //    "#include <fcntl.h>\n"
 //    "int main()\n"
 //    "{\n"
 //    "    int fd = open(\"test\", O_CREAT);\n"
 //    "    while (1)\n"
 //    "        dprintf(fd, \"24\");\n"
 //    "    return 0;\n"
 //    "}\n"
 //    "--------------------------db7a91874317ec7e\r\n"
 //    "Content-Disposition: form-data; name=\"file2\"; filename=\"Makefile\"\r\n"
 //    "Content-Type: application/octet-stream\r\n"
 //    "\r\n"
 //    "NAME\t= webserv\n\n"
 //    "MAKEFLAGS += -s\n\n"
 //    "INCLUDES = -I ./includes\n\n"
 //    "DEF_COLOR = \\033[0;39m\n"
 //    "GRAY = \\033[0;90m\n"
 //    "RED = \\033[0;91m\n"
 //    "GREEN = \\033[0;92m\n"
 //    "YELLOW = \\033[0;93m\n"
 //    "BLUE = \\033[0;94m\n"
 //    "MAGENTA = \\033[0;95m\n"
 //    "CYAN = \\033[0;96m\n"
 //    "WHITE = \\033[0;97m\n\n"
 //    "SRCS\t=\\tsrc/main.cpp \\\n"
 //    "\tsrc/class/Data.cpp \\\n"
 //    "\tsrc/class/ServerSocket.cpp \\\n"
 //    "\tsrc/class/Epoll.cpp \\\n"
 //    "\tsrc/class/HttpRequest.cpp\n\n"
 //    "OBJS\t= $(SRCS:.cpp=.o)\n\n"
 //    "CXX\t= c++\n\n"
 //    "CXXFLAGS\t+= $(INCLUDES) -Wall -Wextra -Werror -g3\n\n"
 //    "RM\t= @rm -rf\n\n"
 //    "CURRENT_DATE\t:= $(shell date +\"%Y-%m-%d %H:%M:%S\")\n\n"
 //    "SRC_COUNT = $(shell echo $(SRCS) | wc -w)\n\n"
 //    "USER1 = bgrosjea\n"
 //    "USER2 = amirloup\n\n"
 //    "ifeq ($(USER), $(USER1))\n"
 //    "    EXECUTABLE = /loadings/loadingbasile\n"
 //    "else ifeq ($(USER), $(USER2))\n"
 //    "    EXECUTABLE = /loadings/loadingantoine\n"
 //    "else\n"
 //    "    $(error Unknown user, please set the correct executable)\n"
 //    "endif\n\n"
 //    "all\t: $(NAME) clean\n\n"
 //    "$(NAME) : $(OBJS)\n"
 //    "    @{ ./$(EXECUTABLE) $(SRC_COUNT) & $(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME) ; wait; }\n"
 //    "    @echo \"$(MAGENTA)Make Done$(DEF_COLOR)\"\n\n"
 //    "clean : \n"
 //    "    $(RM) $(OBJS) \n"
 //    "    @echo \"$(BLUE)OBJS CLEAR MY FRIEND!$(DEF_COLOR)\"\n\n"
 //    "fclean : \n"
 //    "    $(RM) $(NAME) $(OBJS)\n"
 //    "    @echo \"$(MAGENTA)¯|_(ツ)_/¯ I'M SO PROUD OF U!$(DEF_COLOR)\"\n\n"
 //    "re :\tfclean all\n\n"
 //    "ex :\tre clean\n"
 //    "    @echo \"$(GREEN)(•̀ᴗ•́)و ̑̑ALL FINE!(•̀ᴗ•́)و ̑̑$(DEF_COLOR)\"\n"
 //    "    @./webserv\n\n"
 //    "git\t: fclean\n"
 //    "    @$(RM) */a.out a.out\n"
 //    "    @git add . > /dev/null 2>&1\n"
 //    "    @@msg=$${MSG:-\"$(CURRENT_DATE)\"}; git commit -m \"$(USER) $(CURRENT_DATE) $$msg\" > /dev/null 2>&1 \n"
 //    "    @git push > /dev/null 2>&1\n"
 //    "    @echo \"$(GREEN)(•̀ᴗ•́)و ̑̑GIT UPDATE!(•̀ᴗ•́)و ̑̑$(DEF_COLOR)\"\n\n"
 //    "val : re clean\n"
 //    "    @valgrind -s --leak-check=full --show-leak-kinds=all ./webserv\n\n"
 //    "run : \n"
 //    "    @./loadingtriangle $(SRC_COUNT) & wait\n"
 //    "--------------------------db7a91874317ec7e--\r\n";

	HttpRequest rq(dummyRq);
	if (!rq.isValid()) {
		cout << "parsingStrError: " << rq.parsingStrError << endlou;
		cout << "err " << rq.errNo << endl;
		exit(1);
	}
	rq.parseRequest();
	if (rq.parsingError) {
		cout << "parsingStrError: " << rq.parsingStrError << endlou;
		cout << "err " << rq.errNo << endl;
		exit(1);
	}
	headermap_t s = rq.getHeaders();
	for (headermap_t::iterator it = s.begin(); it != s.end(); it++) {
		cout << "Key:" << it->first << endl;
		cout << "rawValue:" << it->second.rawValue << endl;
		int i = 0;
		for (strmap_t::iterator it2 = it->second.parameters.begin(); it2 != it->second.parameters.end(); it2++) {
			cout << "paramKey" << i << ":" << it2->first << endl;
			cout << "paramVal" << i << ":" << it2->second << endl;
			i++;
		}
		cout << endl;
	}
	// cout << "host: |" << rq.getHost() << "|" <<  endlou
	// 	 << "port: |" << rq.getPort() << "|" << endlou
	// 	 << "method: |" << rq.getMethodToString() << "|" << endlou
	// 	 << "url: |" << rq.getUrl() << "|" << endlou
	//      << "httpversion: |" << rq.getHttpVersion() << "|" << endlou;
}
