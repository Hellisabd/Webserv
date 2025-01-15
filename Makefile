NAME	= webserv

MAKEFLAGS += -s

INCLUDES = -I ./includes

DEF_COLOR = \033[0;39m
GRAY = \033[0;90m
RED = \033[0;91m
GREEN = \033[0;92m
YELLOW = \033[0;93m
BLUE = \033[0;94m
MAGENTA = \033[0;95m
CYAN = \033[0;96m
WHITE = \033[0;97m

SRCS	=	src/main.cpp \
			src/class/Data.cpp \
			src/class/ServerSocket.cpp \
			src/class/Epoll.cpp \
			src/class/Client.cpp \
			src/class/HttpRequest.cpp \
			src/class/cgi.cpp \
			src/class/Response.cpp \
			src/uploads.cpp \
			src/login.cpp \
			src/delete.cpp \
			src/dir.cpp
			

OBJS	= $(SRCS:.cpp=.o)

CXX	= c++

CXXFLAGS	+= $(INCLUDES) -Wall -Wextra -Werror -g3 -std=c++98

RM	= @rm -rf

CURRENT_DATE	:= $(shell date +"%Y-%m-%d %H:%M:%S")

all	: $(NAME) clean

$(NAME) : $(OBJS)
	@$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)
	@echo "$(MAGENTA)Make Done$(DEF_COLOR)"

clean : 
	$(RM) $(OBJS) 
	@echo "$(BLUE)OBJS CLEAR MY FRIEND!$(DEF_COLOR)"

fclean : 
	$(RM) $(NAME) $(OBJS)
	@echo "$(MAGENTA)¯|_(ツ)_/¯ I'M SO PROUD OF U!$(DEF_COLOR)"

re :	fclean all

ex :	re clean
	@echo "$(GREEN)(•̀ᴗ•́)و ̑̑ALL FINE!(•̀ᴗ•́)و ̑̑$(DEF_COLOR)"
	@./webserv

git	:	fclean
	@$(RM) */a.out a.out request response /site/downloads/* && touch request response
	@git add . > /dev/null 2>&1
	@@msg=$${MSG:-"$(CURRENT_DATE)"}; git commit -m "$(USER) $(CURRENT_DATE) $$msg" > /dev/null 2>&1 
	@git push > /dev/null 2>&1
	@echo "$(GREEN)(•̀ᴗ•́)و ̑̑GIT UPDATE!(•̀ᴗ•́)و ̑̑$(DEF_COLOR)"

val : re clean
	@valgrind -s --leak-check=full --track-fds=yes --trace-children=yes ./webserv fichierConfig/config

run : 
	@./loadingtriangle $(SRC_COUNT) & wait
