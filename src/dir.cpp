#include <webserv.hpp>

bool isDir(string path) {
    struct stat info;

	path = "." + path;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return S_ISDIR(info.st_mode);
}

void sendDir(string &_response, string dirPath) {
	dirPath = "." + dirPath;
	DIR *dir;
    struct dirent *entry;

    dir = opendir(dirPath.c_str());
    if (dir == NULL) {
        return;
    }
	_response = dirPath + "<br>";
	while ((entry = readdir(dir)) != NULL) {
        string name = entry->d_name;
        if (name == "." || name == "..")
            continue;
        if (entry->d_type == DT_DIR)
            _response += "[DIR] " + name + "<br>";
        else if (entry->d_type == DT_REG)
            _response += "[FILE] " + name + "<br>";
        else
            _response += "[OTHER] " + name + "<br>";
    }
	closedir(dir);
}