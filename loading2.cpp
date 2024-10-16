#include <iostream>
#include <unistd.h>
#define NC		std::string("\e[0m")
#define RED		std::string("\e[1;31m")
#define GREEN	std::string("\e[1;32m")
#define YELLOW	std::string("\e[1;33m")
#define ORANGE	std::string("\e[1;34m")
#define PURPLE	std::string("\e[1;35m")
#define BLUE	std::string("\e[1;36m")

int main()
{
	int size = 10;
	std::string t1 = RED + "▷";
	std::string t2 = BLUE + "◀";
	std::string buffer =  GREEN + "Loading..." + YELLOW + "[";
	for (int j = 0; j < size; j++)
		buffer += " ";
	buffer += YELLOW + "]" + NC;
	std::cout << buffer;
	std::cout.flush();
	for (int i = 12; i < (12 + size); i++)
	{
		buffer.replace(i, 10, t1);
		std::cout << "\r" << buffer;
		std::cout.flush();
		sleep(1);
	}
	std::cout << std::endl;
}

