#include <iostream>
#include <unistd.h>
#include <cstdlib>
#define NC		std::string("\e[0m")
#define RED		std::string("\e[1;31m")
#define GREEN	std::string("\e[1;32m")
#define YELLOW	std::string("\e[1;33m")
#define ORANGE	std::string("\e[1;34m")
#define PURPLE	std::string("\e[1;35m")
#define BLUE	std::string("\e[1;36m")

std::string create_string(int i, int size, bool rev, int nbr_of_triangle)
{
	std::string t1 = RED + "▷";
	std::string t2 = BLUE + "◀";
	std::string t3 = GREEN + "▼";
	std::string t4 = GREEN + "▲";
	std::string buffer =  GREEN + "Loading..." + YELLOW + "[";
	for (int j = 0; j < size; j++)
	{
		if (j < nbr_of_triangle)
		{
			if (j % 2 == 0)
				buffer += t3;
			else
				buffer += t4;
		}
		else if (rev == 0)
		{
			if (i == j)
				buffer += t1;
			else
				buffer += " ";
		}
		else
		{
			if (i == j)
				buffer += t2;
			else
				buffer += " ";
		}
	}
	buffer += YELLOW + "]" + NC;
	return buffer;
}

int main(int argc, char **argv)
{
	(void)argc;
	int size = atol(argv[1]);
	int i = 0;
	int r = 0;
	bool reverse = 0;
	int nbr_of_triangle = 0;
	while (i < size)
	{
		while (1)
		{
			std::cout.flush();
			std::cout << "\r";
			if (r >= size)
				std::cout << create_string(r, size, reverse, nbr_of_triangle);
			else
				std::cout << create_string(r, size, reverse, nbr_of_triangle);
			if (size <= 10)
				usleep(20000);
			else if (size > 10 && size < 20)
				usleep(5000);
			else if (size > 20)
				usleep(1000);
			if (r < size && reverse == 0)
				r++;
			else
			{
				reverse = 1;
				r--;
			}
			if (!r)
				break;
		}
		reverse = 0;
		nbr_of_triangle++;
		i++;
	}
	std::cout.flush();
	std::cout << "\r";
	std::cout << create_string(r, size, reverse, nbr_of_triangle);
	std::cout << std::endl;
}

