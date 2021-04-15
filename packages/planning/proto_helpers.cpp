#include "packages/planning/proto_helpers.h"

#include <fstream>

#include "glog/logging.h"

namespace planning {

std::string loadProtoText(const std::string& file_to_read) {
    CHECK(!file_to_read.empty());
    std::fstream input(file_to_read, std::ios::in);
    CHECK(input.good()) << "Failed to read in: " << file_to_read;
    std::stringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

} // planning
