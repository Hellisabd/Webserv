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

int main(int argc, char **argv)
{
	(void)argc;
	int size = atol(argv[1]);
	if (size < 1)
		return (0);
	int anim = 0;
	string buffer = YELLOW + "Loading..." + BLUE + "[" + YELLOW;
	int pos = size + 13;
	int replaced = buffer.size() - 8;
	for (int i = 0; i < size; i++)
	{
		string buffer2 = buffer;
		if (anim < 2)
		{
			cout << buffer2  << "\033[" << pos << "G" << "]";
			usleep(10000);
			cout.flush();
			cout << "\r\033[K";
			for (; anim < 2; anim++)
			{
				cout << buffer2 << RED + "|";
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
				cout << buffer2 << ORANGE + "/";
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
				if (anim % 2 == 0)
				{
					cout << buffer2 << BLUE + "―";
					usleep(10000);
					cout.flush();
					cout << "\r\033[K";
					cout << buffer2 << GREEN + '\\';
					usleep(10000);
					cout.flush();
					cout << "\r\033[K";
					buffer2 += "\\";
				}
				else
					buffer2 += "/";
				cout << buffer2;
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
			}
			buffer += "▼";
		}
		else if (anim < 4)
		{
			for (; anim < 4; anim++)
			{
				cout << buffer2 << RED + "|";
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
				cout << buffer2 << ORANGE + "/";
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
				if (anim % 2 == 1)
				{
					cout << buffer2 << BLUE + "―";
					usleep(10000);
					cout.flush();
					cout << "\r\033[K";
					cout << buffer2 << GREEN + '\\';
					usleep(10000);
					cout.flush();
					cout << "\r\033[K";
					buffer2 += "\\";
				}
				else
					buffer2 += "/";
				cout << buffer2;
				usleep(10000);
				cout.flush();
				cout << "\r\033[K";
			}
			anim = 0;
			buffer += "▲";
		}
	}

	buffer.replace(0, 7, GREEN);
	buffer.replace(replaced + 1, 7, GREEN);
	cout << buffer << BLUE + "] " + NC << endl;
	return (0);
}