all : 
	c++ example_client.cpp -o client
	c++ example_server.cpp -o server

serv : 
	valgrind --leak-check=full --track-fds=yes ./server

client :
	valgrind --leak-check=full --track-fds=yes ./client