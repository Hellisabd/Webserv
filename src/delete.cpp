#include <webserv.hpp>

void delete_file(std::string path) {
	std::size_t start = path.find("filename=", 0) + 9;
	std::string filename;
	if (start != path.npos) {
		filename = path.substr(start, path.length() - start);
	}
	else
		throw Error("File to delete not found.");
	std::string file_path = "./site/downloads/" + filename;
	if (std::remove(file_path.c_str()) == 0)
        debug(GREEN, filename + " has been deleted successfully.");
    else 
        throw Error("Error deleting file");
}