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
	std::string buffer =  GREEN + "Loading..." + YELLOW + "[";
	int pos = buffer.size() + 1;
	buffer += "▷";
	for (int j = 1; j < size; j++)
		buffer += " ";
	buffer += YELLOW + "]";
	for (int i = 12;  < (12 + size); i++)
	{
		std::cout << buffer << std::endl;
	}
}