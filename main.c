#include <stdio.h>
#include <fcntl.h>
int main()
{
	int fd = open("test", O_CREAT);
	while (1)
		dprintf(fd, "24");
	return 0;
}