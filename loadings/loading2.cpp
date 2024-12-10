#include <iostream>
#include <unistd.h>
#include <cstdlib>
#define NC		string("\e[0m")
#define RED		string("\e[1;31m")
#define GREEN	string("\e[1;32m")
#define YELLOW	string("\e[1;33m")
#define ORANGE	string("\e[1;34m")
#define PURPLE	string("\e[1;35m")
#define BLUE	string("\e[1;36m")

string create_string(int i, int size, bool rev, int nbr_of_triangle)
{
	string t1 = RED + "▷";
	string t2 = BLUE + "◀";
	string t3 = GREEN + "▼";
	string t4 = GREEN + "▲";
	string buffer =  GREEN + "Loading..." + YELLOW + "[";
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
			cout.flush();
			cout << "\r";
			if (r >= size)
				cout << create_string(r, size, reverse, nbr_of_triangle);
			else
				cout << create_string(r, size, reverse, nbr_of_triangle);
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
	cout.flush();
	cout << "\r";
	cout << create_string(r, size, reverse, nbr_of_triangle);
	cout << endl;
}

