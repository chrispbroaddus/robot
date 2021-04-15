#pragma once

#include <string>
#include <vector>

namespace filesystem {
/// ReadFileToString reads the contents of a file and returns them as a string.
/// It throws one of the std::ios exceptions if the file cannot be read.
std::string ReadFileToString(const std::string& path);

/// Get a list of all the files in a directory
std::vector<std::string> getFileList(const std::string& dir);
}
