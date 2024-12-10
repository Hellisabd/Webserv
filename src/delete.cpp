#include <webserv.hpp>

void delete_file(string path, Data &data) {
	(void)data;
	size_t start = path.find("filename=", 0) + 9;
	string filename;
	if (start != path.npos) {
		filename = path.substr(start, path.length() - start);
	}
	else
		throw Error("File to delete not found.");
	string file_path = "./site/downloads/" + filename;
	if (remove(file_path.c_str()) == 0) {
		debug(GREEN, filename + " has been deleted successfully.");
		for (vector<string>::iterator it = data._uploads.begin(); it != data._uploads.end(); it ++) {
			if (filename == *it) {
				it = data._uploads.erase(it);
				break ;
			}
		}
	}
	else 
		debug("Error deleting file");
}