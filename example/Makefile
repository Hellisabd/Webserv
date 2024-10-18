all : 
	cc example_client.c -o client
	cc example_server.c -o server

serv : 
	valgrind --leak-check=full --track-fds=yes ./server

cli :
	valgrind --leak-check=full --track-fds=yes ./client prout

re : 
	rm -rf server