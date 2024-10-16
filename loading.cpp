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
	int anim = 0;
	std::string buffer = GREEN + "Loading..." + YELLOW + "[";
	int pos = size + 13;
	for (int i = 0; i < size; i++)
	{
		std::string buffer2 = buffer;
		if (anim < 2)
		{
			std::cout << buffer2  << "\033[" << pos << "G" << "]";
			usleep(30000);
			std::cout.flush();
			std::cout << "\r";
			for (; anim < 2; anim++)
			{
				std::cout << buffer2 << RED + "|";
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
				std::cout << buffer2 << ORANGE + "/";
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
				if (anim % 2 == 0)
				{
					std::cout << buffer2 << BLUE + "―";
					usleep(30000);
					std::cout.flush();
					std::cout << "\r";
					std::cout << buffer2 << GREEN + '\\';
					usleep(30000);
					std::cout.flush();
					std::cout << "\r";
					buffer2 += "\\";
				}
				else
					buffer2 += "/";
				std::cout << buffer2;
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
			}
			buffer += "▼";
		}
		else if (anim < 4)
		{
			for (; anim < 4; anim++)
			{
				std::cout << buffer2 << RED + "|";
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
				std::cout << buffer2 << ORANGE + "/";
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
				if (anim % 2 == 1)
				{
					std::cout << buffer2 << BLUE + "―";
					usleep(30000);
					std::cout.flush();
					std::cout << "\r";
					std::cout << buffer2 << GREEN + '\\';
					usleep(30000);
					std::cout.flush();
					std::cout << "\r";
					buffer2 += "\\";
				}
				else
					buffer2 += "/";
				std::cout << buffer2;
				usleep(30000);
				std::cout.flush();
				std::cout << "\r";
			}
			anim = 0;
			buffer += "▲";
		}
	}
	std::cout << buffer << YELLOW + "] " + NC << std::endl;
}