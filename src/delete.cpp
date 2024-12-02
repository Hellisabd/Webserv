#include <webserv.hpp>

void delete_file(std::string path, Data &data) {
	(void)data;
	std::size_t start = path.find("filename=", 0) + 9;
	std::string filename;
	if (start != path.npos) {
		filename = path.substr(start, path.length() - start);
	}
	else
		throw Error("File to delete not found.");
	std::string file_path = "./site/downloads/" + filename;
	if (std::remove(file_path.c_str()) == 0) {
		debug(GREEN, filename + " has been deleted successfully.");
		for (std::vector<std::string>::iterator it = data._uploads.begin(); it != data._uploads.end(); it ++) {
			if (filename == *it) {
				it = data._uploads.erase(it);
				break ;
			}
		}
	}
	else 
		debug("Error deleting file");
}