#include <dirent.h>
#include <fstream>
#include <vector>

#include "packages/filesystem/include/filesystem.h"

namespace filesystem {
std::string ReadFileToString(const std::string& path) {
    std::ifstream s(path);
    return std::string(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>());
}

std::vector<std::string> getFileList(const std::string& dir) {
    DIR* dpdf;
    struct dirent* epdf;
    std::vector<std::string> files;

    dpdf = opendir(dir.c_str());
    if (dpdf != NULL) {
        while ((epdf = readdir(dpdf))) {
            std::string filename(epdf->d_name);
            files.push_back(filename);
        }
    }

    return files;
}
}
